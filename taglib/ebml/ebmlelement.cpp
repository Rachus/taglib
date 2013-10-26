/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include "ebmlfile.h"

using namespace TagLib;

class EBML::Element::ElementPrivate
{
public:

  // The file that contains this element.
  EBML::File* document;

  // The parent element.
  Element *parent;

  // The child elements.
  List<Element *> children;

  // The id of the element (comparable to the tag name in a XML document)
  ulli id;

  // Position of the element's header.
  offset_t position;

  // The size of the element (read from the header).
  // Actually an ulli but due to the variable integer size limited, thus
  // offset_t is also a valid type.
  offset_t size;

  // The position of the element's data.
  offset_t data;

  // True if this element created all children.
  bool populated;

  // False if an error occured.
  bool valid;

  // Reading constructor: Initialises this element and reads its header.
  ElementPrivate(EBML::File *doc, Element *par, offset_t pos) :
    document(doc),
    parent(par),
    position(pos),
    populated(false),
    valid(false)
  {
    readElement();
  }

  // Writing constructor: Initialises this element and adds it to the list of
  // the parent's children.
  ElementPrivate(EBML::File *doc, Element *par, offset_t pos, ulli i, offset_t s) :
    document(doc),
    parent(par),
    id(i),
    position(pos),
    size(s),
    populated(true),
    valid(false)
  {
    // TODO
  }

  ~ElementPrivate()
  {
    // Clean up every single child element.
    for(List<Element *>::Iterator i = children.begin(); i != children.end(); ++i) {
      delete *i;
    }
  }

  // Reads a variable length integer from the file at the given position and
  // saves its value to result. If cutOne is true, the first one in the binary
  // representation of the result is removed (required for size). If cutOne is
  // false the one will remain in the result (required for id). This method
  // returns the position directly after the read integer, or a negativ value,
  // if an error occured.
  offset_t readVInt(offset_t position, ulli *result, bool cutOne = true)
  {
    document->seek(position);

    // Determine the length of the integer by analysing the first byte
    ByteVector firstBlock = document->readBlock(1);
    if(firstBlock.size() != 1)
		  return -1L;
    char firstByte = firstBlock[0];
    uint byteAmount = 1;
    for(uint i = 0; i < 8 && ((firstByte << i) & (1 << 7)) == 0; ++i)
      ++byteAmount;

    // Read the variable length integer
    document->seek(position);
    ByteVector vint = document->readBlock(byteAmount);
    if(vint.size() != byteAmount)
      return -1L;

    // Remove the first one if requested.
    if(cutOne)
      vint[0] = vint[0] & (~(1 << (8 - byteAmount)));

    // Store result and return ne position
    if(result)
      *result = static_cast<ulli>(vint.toInt64BE(0));
    return position + byteAmount;
  }

  // Returns a ByteVector containing the given number as a variable integer.
  // Truncates numbers > 2^56 (^ means potency in this case).
  // If addOne is true, the ByteVector will add the one that determines the
  // length of the integer. Otherwise the user is responsible for adding the
  // correct size information to the first byte. (Element identification
  // numbers are usually given in this format)
  // If shortest is true, the ByteVector will be as short as possible
  // (required for the id).
  ByteVector createVInt(ulli number, bool addOne = true, bool shortest = true)
  {
    ByteVector vint = ByteVector::fromUInt64BE(number);

    // Just prepend 0b 0000 0001 if necessary and return the vint, if not the
    // shortest representation is requested.
    if(!shortest) {
      if(addOne)
        vint[0] = 1;
      return vint;
    }

    // Calculate the minimal length of the integer.
    size_t byteAmount = vint.size();
    for(size_t i = 0; byteAmount > 0 && vint[i] == 0; ++i)
      --byteAmount;

    // Ready if no one must be added.
    if(!addOne)
      return ByteVector(vint.data() + vint.size() - byteAmount, byteAmount);

    ulli firstByte = 1 << (vint.size() - byteAmount);

    // The most significant byte loses #byteAmount bits for storing the
    // integer length. Therefore we might need to increase the byteSize.
    if(number >= (firstByte << (8 * (byteAmount - 1))) && byteAmount < vint.size())
      ++byteAmount;

    // Add the one at the correct position.
    size_t firstBytePosition = vint.size() - byteAmount;
    vint[firstBytePosition] |= 1 << firstBytePosition;
    return ByteVector(vint.data() + firstBytePosition, byteAmount);
  }

  // Creates a ByteVector containing the header of an element with given id.
  // The complete size available for the element must be given as fullsize.
  // Returns null, if there is not enough space.
  ByteVector makeHeader(ulli id, offset_t fullsize)
  {
    ByteVector header = createVInt(id, false);
    offset_t newsize = fullsize - header.size() - 8;
    if(newsize < 0)
      return ByteVector::null;
    ByteVector size = createVInt(newsize);
    header.append(size);
    return header;
  }

  // Reads the element's header (id and size). Sets the valid flag to true, if
  // successfull and to false otherwise.
  void readElement()
  {
    if(parent) {
      data = readVInt(position, &id, false);
      if(data < 0) {
        valid = false;
        return;
      }
      ulli s;
      data = readVInt(data, &s);
      size = static_cast<offset_t>(s);
      valid = true;
    }
    else {
      document->seek(0, File::End);
      size = document->tell();
    }
  }

  // Converts this element to a void element and merges elements before and
  // after the current one.
  //
  // Approach:
  // 1. Change the ID within the header to EBML::Void
  // 2. Merge:
  //    a) Search elements before and after the current element with the same
  //       parent.
  //    b) If one of the elements found in the step before is an void element,
  //       merge the found elements.
  //    c) Jump to a) if there was a merge.
  void makeVoid()
  {
    // 1.:
    offset_t fullsize = size + data - position;
    // Should fit in the place where the old header was.
    ByteVector voidHeader = makeHeader(id, fullsize);
    document->seek(position);
    document->write(voidHeader);
    id = Void;
    size = fullsize - voidHeader.size();
    data = position + voidHeader.size();
    populated = true;
    valid = true;

    // 2.:
    if(parent) {
      offset_t afterEnd = data + size;
      bool foundVoid = false;
      do {
        // a):
        for(List<Element *>::Iterator i = children.begin(); i != children.end(); ++i) {
          Element *current = *i;
          if(current->position == afterEnd) {
            // [...][V][current][...]
          }
          else if(position == current->data + current->size) {
            // [...][current][V][...]
          }
        }
      } while(foundVoid);
    }
    /*
    if(parent) {
      // Also remove all children.
      children.clear();
      id = Void;

      offset_t next_pos = data + size;
      Element* next_void;
      ByteVector header;
      offset_t fullsize;
      do {
        next_void = 0;
        // Void elements have the same parent and document.
        for(List<Element *>::Iterator i = children.begin(); i != children.end(); ++i) {
          Element *current = *i;
          // Case: This is the first element.
          if(current->position == next_pos) {
            next_void = *i;
            fullsize = next_pos + current->size - position;
            header = makeHeader(id, fullsize);
            if(header == ByteVector::null)
              return;
            data = position + header.size();
            break;
          }
          // Case: This is the second element.
          else if(position == current->data + current->size) {
            next_void = *i;
            fullsize = next_pos - current->position;
            header = makeHeader(id, fullsize);
            if(header == ByteVector::null)
              return;
            position = current->position;
            data = position + header.size();
            break;
          }
        }
        size = fullsize - header.size();
        data = position + header.size();
        populated = true;
        document->seek(position);
        document->writeBlock(header);
        valid = true;
        // Clear ressources of the old element.
//        parent->
      } while(next_void);
    }*/
  }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::Element::~Element()
{
  delete d;
}

EBML::Element::Element(EBML::File *document)
  : d(new EBML::Element::ElementPrivate(document, 0, 0))
{
}

EBML::Element *EBML::Element::getChild(EBML::ulli id)
{
  populate();
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i) {
    if((*i)->d->id == id)
      return *i;
  }
  return 0;
}

List<EBML::Element *> EBML::Element::getChildren(EBML::ulli id)
{
  populate();
  List<Element *> result;
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i) {
    if((*i)->d->id == id)
      result.append(*i);
  }
  return result;
}

List<EBML::Element *> EBML::Element::getChildren()
{
  populate();
  // Copy each element to a new list. Since Lists are shared, the user cannot
  // manipulate the private list.
  List<Element *> result;
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i) {
    result.append(*i);
  }
  return result;
}

EBML::Element *EBML::Element::getParent()
{
  return d->parent;
}

ByteVector EBML::Element::getAsBinary()
{
  d->document->seek(d->data);
  return d->document->readBlock(static_cast<size_t>(d->size));
}

String EBML::Element::getAsString()
{
  return String(getAsBinary(), String::UTF8);
}

signed long long EBML::Element::getAsInt()
{
  return getAsBinary().toInt64BE(0);
}

EBML::ulli EBML::Element::getAsUnsigned()
{
  return static_cast<ulli>(getAsBinary().toInt64BE(0));
}

long double EBML::Element::getAsFloat()
{
  const ByteVector bin = getAsBinary();
  switch(bin.size()) {
  case 4:
    return bin.toFloat32BE(0);
  case 8:
    return bin.toFloat64BE(0);
  case 10:
    return bin.toFloat80BE(0);
  default:
    return 0.0;
  }
}

EBML::Element *EBML::Element::addElement(EBML::ulli id)
{
  populate();
  return new Element(d->document, this, d->data + d->size, id, 0);
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, const ByteVector &binary)
{
  populate();
  return new Element(d->document, this, d->data + d->size, id, binary.size());
}

EBML::Element *EBML::Element::addElement(ulli id, const String &string)
{
  return addElement(id, string.data(String::UTF8));
}

EBML::Element *EBML::Element::addElement(ulli id, signed long long number)
{
  return addElement(id, ByteVector::fromUInt64BE(number));
}

EBML::Element *EBML::Element::addElement(ulli id, EBML::ulli number)
{
  return addElement(id, ByteVector::fromUInt64BE(number));
}

// addElement with float is not necessary.

bool EBML::Element::removeChildren(EBML::ulli id, bool useVoid)
{
  bool result = false;
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i) {
    if((*i)->d->id == id) {
      removeChild(*i, useVoid);
      result = true;
    }
  }
  return result;
}

bool EBML::Element::removeChildren(bool useVoid)
{
  if(d->children.isEmpty())
    return false;

  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i) {
    removeChild(*i, useVoid);
  }
  return true;
}

bool EBML::Element::removeChild(Element *element, bool useVoid)
{
  if(!d->children.contains(element))
    return false;
  if(useVoid) {
    d->makeVoid();
  }
  else {

  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

EBML::Element::Element(EBML::File *document, Element *parent, offset_t pos)
  : d(new EBML::Element::ElementPrivate(document, parent, pos))
{
}

EBML::Element::Element(EBML::File *document, Element *parent, offset_t pos, EBML::ulli id,  offset_t size)
  : d(new EBML::Element::ElementPrivate(document, parent, pos, id, size))
{
}

void EBML::Element::populate()
{
  if(!d->populated) {
    d->populated = true;
    offset_t end = d->data + d->size;

    for(offset_t i = d->data; i < end;) {
      Element *elem = new Element(d->document, this, i);
      if (!elem->valid()) {
        delete elem;
        continue;
      }
      d->children.append(elem);
      i = elem->d->data + elem->d->size;
    }
  }
}


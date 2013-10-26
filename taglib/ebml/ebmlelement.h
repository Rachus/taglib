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

#ifndef TAGLIB_EBMLELEMENT_H
#define TAGLIB_EBMLELEMENT_H

#include "ebmlconstants.h"
#include "tlist.h"
#include "tbytevector.h"
#include "tstring.h"

namespace TagLib {

  namespace EBML {

    class File;

    /*!
     * 
     */
    class TAGLIB_EXPORT Element
    {
    public:
      /*!
       * Create a root element by reading the beginning of the document.
       */
      Element(EBML::File* document);
      ~Element();

      /*!
       * Returns true if this Element is correctly initialized and could read
       * all necessary information or false otherwise.
       */
      bool valid();

      /*!
       * Returns the first found child element with the given id or a null
       * pointer if the child does not exist.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *getChild(const ulli id);

      /*! Returns a list of all child elements with the given id or an empty
       * list if no such element exists.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      List<Element *> getChildren(const ulli id);

      /*!
       * Returns a list of all available child elements or an empty list if
       * there are no children.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      List<Element *> getChildren();

      /*!
       * Returns the parent element or null if no such element exists.
       */
      Element *getParent();

      /*!
       * Returns the raw content of this element.
       */
      ByteVector getAsBinary();

      /*!
       * Returns the content of this element interpreted as a string.
       */
      String getAsString();

      /*!
       * Returns the content of this element interpreted as signed integer.
       *
       * Do not call this method if *this element is not an INT element (see
       * corresponding DTD)
       */
      signed long long getAsInt();

      /*!
       * Returns the content of this element interpreted as unsigned integer.
       *
       * Do not call this method if *this element is not an UINT element (see
       * corresponding DTD)
       */
      ulli getAsUnsigned();

      /*!
       * Returns the content of this element interpreted as a floating point
       * type. The value is only valid, if the element contained 32, 64 or 80
       * bits.
       *
       * Do not call this method if *this element is not a FLOAT element (see
       * corresponding DTD)
       */
      long double getAsFloat();

      /*!
       * Adds an empty element with given id to this element. Returns a
       * pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id);

      /*!
       * Adds a new element, containing the given binary, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id, const ByteVector &binary);

      /*!
       * Adds a new element, containing the given string, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id, const String &string);

      /*!
       * Adds a new element, containing the given integer, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id, signed long long number);

      /*!
       * Adds a new element, containing the given unsigned integer, to this
       * element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id, ulli number);

      /*!
       * Adds a new element, containing the given floating point value, to
       * this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      Element *addElement(ulli id, long double number);

      /*!
       * Removes all children with the given id. Returns false if there was no
       * such element. Every pointer to a removed element is invalidated.
       * If useVoid is true, the element(s) will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      bool removeChildren(ulli id, bool useVoid = true);

      /*!
       * Removes all children. Returns false if there was no such element.
       * Every pointer to a removed element is invalidated.
       * If useVoid is true, the element(s) will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      bool removeChildren(bool useVoid = true);

      /*!
       * Removes the given element and invalidates its pointer.
       * If useVoid is true, the element will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element
       * (see corresponding DTD)
       */
      bool removeChild(Element *element, bool useVoid = true);

      /*!
       * Writes the given binary to this element.
       */
      void setAsBinary(const ByteVector &binary);

      /*!
       * Writes the given string to this element.
       */
      void setAsString(const String &string);

      /*!
       * Writes the given signed integer to this element.
       */
      void setAsInt(signed long long number);

      /*!
       * Writes the given unsigned integer to this element.
       */
      void setAsUnsigned(ulli number);

      //void setAsFloat(long double number); // Currently irrelevant

    private:
      /*!
       * Creates a new child element of parent in the given document. Tries
       * to parse the information found at pos within the file.
       */
      Element(EBML::File *document, Element *parent, offset_t pos);

      /*!
       * Creates a new child element of parent in the given document. Writes
       * the new header to the file and allocates size bytes of space to the
       * element.
       */
      Element(EBML::File *document, Element *parent, offset_t pos, EBML::ulli id, offset_t size);

      /*!
       * Lazy parsing. This method will be triggered when trying to access
       * children.
       * It is necessary to be a method of Element because we need access to
       * the new created children.
       */
      void populate();

      Element(const Element &);
      Element &operator=(const Element &);

      class ElementPrivate;
      ElementPrivate *d;
    };
  }
}

#endif

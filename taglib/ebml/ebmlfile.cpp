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

class EBML::File::FilePrivate
{
public:
  FilePrivate(EBML::File* document) : root(document) {}
  Element root;

  bool fileValid(EBML::File* document)
  {
    if(!root.valid())
      return false;

    // Sanity check: Is the magical number correct?
    ByteVector magical = document->readBlock(4);
    if(static_cast<ulli>(magical.toUInt32BE(0)) != Header::EBML) {
      return false;
    }
    // Sanity check: Is this a suitable EBML version?
    Element *head = root.getChild(Header::EBML), *p;
    if(!head ||
      !((p = head->getChild(Header::EBMLVersion)) && p->getAsUnsigned() == 1L) ||
      !((p = head->getChild(Header::EBMLReadVersion)) && p->getAsUnsigned() == 1L) ||
      // The standard actually only supports EBMLMaxIDWidth up to 4.
      !((p = head->getChild(Header::EBMLMaxIDWidth)) && p->getAsUnsigned() <= 8) ||
      !((p = head->getChild(Header::EBMLMaxSizeWidth)) && p->getAsUnsigned() <= 8)
    ) {
      return false;
    }
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::File::~File()
{
  delete d;
}

EBML::Element *EBML::File::getDocumentRoot()
{
  return &d->root;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////
EBML::File::File(FileName file) : TagLib::File(file)
{
  d = new FilePrivate(this);
  if(!d->fileValid(this))
    setValid(false);
}

EBML::File::File(IOStream *stream) : TagLib::File(stream)
{
  d = new FilePrivate(this);
  if(!d->fileValid(this))
    setValid(false);
}


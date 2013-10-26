
#include <ebmlfile.h>
#include <cppunit/extensions/HelperMacros.h>

#include "utils.h"

using namespace TagLib;

// This class makes EBML::File usable (it's actually abstract)
class UsableEBMLFile : public TagLib::EBML::File
{
public:
  UsableEBMLFile(FileName file) : TagLib::EBML::File(file) {}
  bool save() { return false; }
  Tag *tag() const { return 0; }
  AudioProperties *audioProperties() const { return 0; }
private:
  UsableEBMLFile(const UsableEBMLFile &);
};

class TestEBML : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestEBML);
  CPPUNIT_TEST(testOpenEmptyFile);
  CPPUNIT_TEST(testAddAndRemoveElements);
  CPPUNIT_TEST_SUITE_END();

public:

  void testOpenEmptyFile()
  {
    ScopedFileCopy copy("completely_empty", "ebml");
    UsableEBMLFile ef(copy.fileName().c_str());
    CPPUNIT_ASSERT_EQUAL(true, ef.isValid());
  }

  void testAddAndRemoveElements()
  {
    ScopedFileCopy copy("completely_empty", "ebml");
    UsableEBMLFile ef(copy.fileName().c_str());

    EBML::Element *root = ef.getDocumentRoot();
    CPPUNIT_ASSERT_EQUAL(true, root != 0);

	// Add

	// and check
  }
};

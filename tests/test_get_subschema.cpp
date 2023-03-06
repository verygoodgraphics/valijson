#include <gtest/gtest.h>

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/validator.hpp>

using namespace std;
using namespace valijson;
using namespace valijson::adapters;

#define TEST_DATA_DIR "../tests/vgg/"

int fake_main(string schema_path, string target_document_path);

TEST(TestGetSubSchema, Smoke)
{
    auto result =
        fake_main(TEST_DATA_DIR "vgg-format.json", TEST_DATA_DIR "2020.json");

    EXPECT_EQ(result, 0);
}

int fake_main(string schema_path, string target_document_path)
{
    // Load the document containing the schema
    nlohmann::json schemaDocument;
    if (!valijson::utils::loadDocument(schema_path, schemaDocument)) {
        cerr << "Failed to load schema document." << endl;
        return 1;
    }

    // Load the document that is to be validated
    nlohmann::json targetDocument;
    if (!valijson::utils::loadDocument(target_document_path, targetDocument)) {
        cerr << "Failed to load target document." << endl;
        return 1;
    }

    // Parse the json schema into an internal schema format
    Schema schema;
    SchemaParser parser;
    NlohmannJsonAdapter schemaDocumentAdapter(schemaDocument);
    try {
        parser.populateSchema(schemaDocumentAdapter, schema);
    } catch (std::exception &e) {
        cerr << "Failed to parse schema: " << e.what() << endl;
        return 1;
    }

    // Perform validation
    Validator validator(Validator::kStrongTypes);
    ValidationResults results;
    NlohmannJsonAdapter targetDocumentAdapter(targetDocument);
    if (!validator.validate(schema, targetDocumentAdapter, &results)) {
        std::cerr << "Validation failed." << endl;
        ValidationResults::Error error;
        unsigned int errorNum = 1;
        while (results.popError(error)) {

            std::string context;
            std::vector<std::string>::iterator itr = error.context.begin();
            for (; itr != error.context.end(); itr++) {
                context += *itr;
            }

            cerr << "Error #" << errorNum << std::endl
                 << "  context: " << context << endl
                 << "  desc:    " << error.description << endl;
            ++errorNum;
        }
        return 1;
    }

    return 0;
}

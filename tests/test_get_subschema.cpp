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
// #define TEST_DATA_DIR "../../tests/vgg/"

int validate_json(const Subschema &schema,
                  NlohmannJsonAdapter &target_docment_adapter);
int load_schema(string schema_path, Schema &schema);
int load_vgg_schema(Schema &schema);
nlohmann::json load_target_document(string target_document_path);

TEST(TestGetSubSchema, Smoke)
{
    Schema schema;
    load_vgg_schema(schema);

    auto target = load_target_document(TEST_DATA_DIR "2020.json");
    NlohmannJsonAdapter targetDocumentAdapter(target);
    auto result = validate_json(schema, targetDocumentAdapter);

    EXPECT_EQ(result, 0);
}

TEST(TestGetSubSchema, GetColorSubschema)
{
    Schema schema;
    load_vgg_schema(schema);

    auto result = schema.getSubschemaByTitle("Color");
    EXPECT_TRUE(result != nullptr);
}

TEST(TestGetSubSchema, ValidateJsonAgainstSubschema)
{
    Schema schema;
    load_vgg_schema(schema);

    auto color_subschema = schema.getSubschemaByTitle("Color");
    EXPECT_TRUE(color_subschema != nullptr);

    if (color_subschema) {
        auto target = load_target_document(TEST_DATA_DIR "color.json");
        NlohmannJsonAdapter targetDocumentAdapter(target);
        auto result = validate_json(*color_subschema, targetDocumentAdapter);

        EXPECT_EQ(result, 0);
    }
}

int load_vgg_schema(Schema &schema)
{
    return load_schema(TEST_DATA_DIR "vgg-format.json", schema);
}

int load_schema(string schema_path, Schema &schema)
{
    // Load the document containing the schema
    nlohmann::json schemaDocument;
    if (!valijson::utils::loadDocument(schema_path, schemaDocument)) {
        cerr << "Failed to load schema document." << endl;
        return 1;
    }

    // Parse the json schema into an internal schema format
    SchemaParser parser;
    NlohmannJsonAdapter schemaDocumentAdapter(schemaDocument);
    try {
        parser.populateSchema(schemaDocumentAdapter, schema);
    } catch (std::exception &e) {
        cerr << "Failed to parse schema: " << e.what() << endl;
        return 1;
    }

    return 0;
}

nlohmann::json load_target_document(string target_document_path)
{
    // Load the document that is to be validated
    nlohmann::json targetDocument;
    if (!valijson::utils::loadDocument(target_document_path, targetDocument)) {
        throw std::runtime_error("Failed to load target document.");
    }

    return targetDocument;
}

int validate_json(const Subschema &schema,
                  NlohmannJsonAdapter &target_docment_adapter)
{
    // Perform validation
    Validator validator(Validator::kStrongTypes);
    ValidationResults results;
    if (!validator.validate(schema, target_docment_adapter, &results)) {
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

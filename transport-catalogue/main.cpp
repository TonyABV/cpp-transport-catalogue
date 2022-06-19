#include "json_reader.h"
#include "serialization.h"

#include <iostream>
#include <fstream>
#include <string_view>

using namespace std;

void CreateAndSerializeBase() {
    transport_db::TransportCatalogue catalog;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalog);
    serialization::Serialization serialization(catalog, renderer, router);
    json_reader::JsonReader json(catalog, renderer, router, serialization);

    transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);


   ifstream input("make_base.json");

    json.ReadInput(input);
    json.FillCatalogue();
    router.GenerateRouter();
    request_handler.SerializeBase();
}

void DeserializeBaseAndCreate() {
    transport_db::TransportCatalogue catalog;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalog);
    serialization::Serialization serialization(catalog, renderer, router);
    json_reader::JsonReader json(catalog, renderer, router, serialization);

    ofstream output("output.json");
    ifstream input("process_requests.json");/**/

    transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
    json.ReadInput(input);
    serialization.Deserialize();
    json.PrintRequests(output, request_handler);
}

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    /*if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {*/
        // make base here
        CreateAndSerializeBase();
    /*}
    else if (mode == "process_requests"sv) {*/
        // process requests here
        DeserializeBaseAndCreate();
    /*}
    else {
        PrintUsage();
        return 1;
    }*/
}

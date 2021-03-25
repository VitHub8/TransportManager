#include "json.h"
#include "response.h"
#include "transport_manager.h"
#include "request.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

void TestDist() {
    TransportManager tm;
    ostringstream ss_o;

    string input =
            "Bus Looped_Single_Intrastop_Distance: A > B > C > A\n"
            "Stop A: 55.574371, 37.6517, 100m to B\n"
            "Stop B: 55.581065, 37.64839, 200m to C\n"
            "Stop C: 55.587655, 37.645687, 300m to A\n"

    "Bus Looped_Double_Intrastop_Distance: D > E > F > D\n"
            "Stop D: 55.574371, 37.6517, 100m to E, 400m to F\n"
            "Stop E: 55.581065, 37.64839, 200m to F, 500m to D\n"
            "Stop F: 55.587655, 37.645687, 300m to D, 600m to E\n"

    "Bus Straight_Single_Intrastop_Distance: G - H - I\n"
            "Stop G: 55.574371, 37.6517, 100m to H\n"
            "Stop H: 55.581065, 37.64839, 200m to I\n"
            "Stop I: 55.587655, 37.645687, 300m to H\n"

    "Bus Straight_Double_Intrastop_Distance: J - K - L\n"
            "Stop J: 55.574371, 37.6517, 100m to K, 400m to L\n"
            "Stop K: 55.581065, 37.64839, 200m to L, 500m to J\n"
            "Stop L: 55.587655, 37.645687, 300m to J, 600m to K\n"
    ;

    istringstream is1(to_string(16) +"\n"+ input);

    string query =
            "Bus Looped_Single_Intrastop_Distance\n"
            "Bus Looped_Double_Intrastop_Distance\n"
            "Bus Straight_Single_Intrastop_Distance\n"
            "Bus Straight_Double_Intrastop_Distance"
    ;

    istringstream is2(to_string(4) +"\n"+ query);

    const auto update_requests = ReadRequests(UPD_OR_READ::UPD, is1);
    ProcessRequests(tm, update_requests);
    const auto read_requests = ReadRequests(UPD_OR_READ::READ, is2);
    const auto responses = ProcessRequests(tm, read_requests);
    PrintResponses(responses, ss_o);

    ASSERT_EQUAL(ss_o.str(),
                 "Bus Looped_Single_Intrastop_Distance: 4 stops on route, 3 unique stops, 600 route length, 0.196736 curvature\n"
                 "Bus Looped_Double_Intrastop_Distance: 4 stops on route, 3 unique stops, 600 route length, 0.196736 curvature\n"
                 "Bus Straight_Single_Intrastop_Distance: 5 stops on route, 3 unique stops, 700 route length, 0.229497 curvature\n"
                 "Bus Straight_Double_Intrastop_Distance: 5 stops on route, 3 unique stops, 1400 route length, 0.458993 curvature\n")
}


int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);

    TransportManager tm;
    try {
        Json::PrintDoc(cout, ConvertToJson(ProcessRequests(tm, Json::ReadRequests(Json::Load(cin)))));
    } catch (exception& ex){
        cerr<< ex.what()<<endl;
    }
//    Json::Document doc = Json::Load(cin);
//    Json::PrintDoc(cout, doc);
//    console_stream
//    const auto update_requests = ReadRequests(UPD_OR_READ::UPD, cin);
//    ProcessRequests(tm, update_requests);
//    const auto read_requests = ReadRequests(UPD_OR_READ::READ, cin);
//    const auto responses = ProcessRequests(tm, read_requests);
//    PrintResponses(responses, cout);


    return 0;
}

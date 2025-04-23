#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <vector>
#include <thread>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <sstream>  // For stringstream to replace fmt::format

// Function to handle CURL responses
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// URL encoding function
std::string encode_url(const std::string &str) {
    std::string encoded = "";
    for (unsigned char c : str) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            encoded += c;
        } else {
            std::stringstream ss;
            ss << std::uppercase << std::hex << (int)c;
            encoded += "%" + ss.str();
        }
    }
    return encoded;
}

// Function to perform BFS using CURL and RapidJSON
void bfs(const std::string &startNode, int depth, int numThreads) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::unordered_map<std::string, bool> visited;
    std::queue<std::string> nodesQueue;
    nodesQueue.push(startNode);
    visited[startNode] = true;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    while (!nodesQueue.empty() && depth > 0) {
        std::string currentNode = nodesQueue.front();
        nodesQueue.pop();

        // Construct the URL
        std::string url = "https://bridges-data.cise.ufl.edu/api/actors_movies/" + encode_url(currentNode);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL failed: " << curl_easy_strerror(res) << std::endl;
            continue;
        }

        // Parse the JSON response
        rapidjson::Document document;
        if (document.Parse(readBuffer.c_str()).HasParseError()) {
            std::cerr << "JSON parse error for node: " << currentNode << std::endl;
            std::cerr << "Response: " << readBuffer << std::endl;
            continue;
        }

        // Assuming the response contains an array of connected nodes
        if (document.HasMember("actors") && document["actors"].IsArray()) {
            const rapidjson::Value& actors = document["actors"];
            for (rapidjson::SizeType i = 0; i < actors.Size(); ++i) {
                std::string neighbor = actors[i].GetString();
                if (visited.find(neighbor) == visited.end()) {
                    visited[neighbor] = true;
                    nodesQueue.push(neighbor);
                }
            }
        }

        --depth;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <startNode> <maxDepth> <numThreads>" << std::endl;
        return 1;
    }

    std::string startNode = argv[1];
    int maxDepth = std::stoi(argv[2]);
    int numThreads = std::stoi(argv[3]);

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(bfs, startNode, maxDepth, numThreads));
    }

    for (auto &t : threads) {
        t.join();
    }

    return 0;
}

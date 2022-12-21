#pragma once
#include <string>

/**
 * Contains configuration constants shared in coordinator and workers
*/
namespace config {
    // Number of aggregates/ range buckets for hash ranging
    static const int nAggregates = 10;
    static const std::string resultFilename = "result_file";

    /**
     * Get your credentials via
     * az storage account list
     * az account get-access-token --resource https://storage.azure.com/ -o tsv --query accessToken
    */
    namespace credentials {
        // Name of the storage account
        static const std::string accountName = "";
        // Access token for Azure storage
        static const std::string accountToken = "";
    }
}


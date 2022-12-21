#pragma once
#include <string>

enum HashMethod { BOOST, CUSTOM };


/**
 * Contains configuration constants shared in coordinator and workers
*/
namespace config {
    // Number of aggregates/ range buckets for hash ranging
    const int nInitialPartitions = 100;
    static const int nAggregates = 5;
    static const HashMethod hashmethod = HashMethod::BOOST; 

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


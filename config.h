#pragma once
#include <string>

enum HashMethod { BOOST, CUSTOM };

enum IO_TYPE { AZURE_BLOB, LOCAL};

/**
 * Contains configuration constants shared in coordinator and workers
*/
namespace config {
    // Number of aggregates/ range buckets for hash ranging
    const int nInitialPartitions = 100;
    static const int nAggregates = 5;
    static const HashMethod hashmethod = HashMethod::BOOST; 
    const bool io_type = IO_TYPE::AZURE_BLOB;

    // logging
    const bool logging = false;
    const bool time_measures_logging = true;

    /**
     * Get your credentials via
     * az storage account list
     * az account get-access-token --resource https://storage.azure.com/ -o tsv --query accessToken
    */
    namespace credentials {
        // Name of the storage account
        static const std::string accountName = "storageforcoursework";
        // Access token for Azure storage
        static const std::string accountToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyIsImtpZCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyJ9.eyJhdWQiOiJodHRwczovL3N0b3JhZ2UuYXp1cmUuY29tLyIsImlzcyI6Imh0dHBzOi8vc3RzLndpbmRvd3MubmV0LzVkN2I0OWU5LTUwZDItNDBkYy1iYWIxLTE0YTJkOTAzNTQyYy8iLCJpYXQiOjE2NzE2NTI5MzAsIm5iZiI6MTY3MTY1MjkzMCwiZXhwIjoxNjcxNjU3Mzk4LCJhY3IiOiIxIiwiYWlvIjoiQVZRQXEvOFRBQUFBUzNDVGE3M2JFS2xRNDd4ZHdYSUIrLzJjNVFEY3huSVhXbHdNeDBmWlBZT3ZLNHRNSFpLa0Y3ZzdXQ1VqR1YvVFlaRFV1TnpmVXVMaGlyallVNTA1K1NBS1ZiSWx6UzRvZ1orV1pBVzdFakE9IiwiYW1yIjpbInB3ZCIsIm1mYSJdLCJhcHBpZCI6IjA0YjA3Nzk1LThkZGItNDYxYS1iYmVlLTAyZjllMWJmN2I0NiIsImFwcGlkYWNyIjoiMCIsImZhbWlseV9uYW1lIjoiU2NobWlkIiwiZ2l2ZW5fbmFtZSI6IkRhbmllbCBCaW4iLCJncm91cHMiOlsiMDhlZmQ5YjItN2ExMS00NjZmLWI5NTktMThiNmMzYzQzZjNiIiwiZDAxMmU3YzctYjBiYi00NGE0LTgzMjQtZTYwZDU3ZTg1OWEyIiwiNWYxNTdhZjAtNjIzOS00OTE1LTllYmEtNDQyMGFiMTI4OTBjIl0sImlwYWRkciI6IjQ2LjEyOC4yNDUuMTM3IiwibmFtZSI6IlNjaG1pZCwgRGFuaWVsIEJpbiIsIm9pZCI6IjYyYzJjNjg2LWIxNzAtNDg3OC04ZWIzLTAwYzE5ZDhhYzA0ZSIsIm9ucHJlbV9zaWQiOiJTLTEtNS0yMS0xNDk5MjYxNzI3LTU1MTc2MTAyLTM1Mjk1MDk5MjktMTAzNjQ3NCIsInB1aWQiOiIxMDAzMjAwMjU1RDBFQkIxIiwicmgiOiIwLkFYUUE2VWw3WGRKUTNFQzZzUlNpMlFOVUxJR21CdVRVODZoQ2tMYkNzQ2xKZXZGMEFMQS4iLCJzY3AiOiJ1c2VyX2ltcGVyc29uYXRpb24iLCJzdWIiOiI0cnRhWXdrdUVZOTNTeHBZbldFMTdNVld0RVF1Z3k5SlV2bXZOVkNMbWRvIiwidGlkIjoiNWQ3YjQ5ZTktNTBkMi00MGRjLWJhYjEtMTRhMmQ5MDM1NDJjIiwidW5pcXVlX25hbWUiOiJkYW5pZWxiaW4uc2NobWlkQHR1bS5kZSIsInVwbiI6ImRhbmllbGJpbi5zY2htaWRAdHVtLmRlIiwidXRpIjoibmt6X2xmbGJLRS1XY291WlY1dkZBQSIsInZlciI6IjEuMCJ9.Mi3stbVQtMYT5hhXRBZkf9n68LFwuO81BCc3_vYxC92xhYwo5RuWDYOkmY-laBjw7rcBIldPrHAM3AXpCeTkG0fmTQ3seWEtNCd7N_JQdclaVgDMG7PugdfMj6D1MvMETHWnJW5CVcvYpakmqR_R-x5Rv99liI_-EHBzL6u4EhGwaHuLhJfTcPr-YxtJX0xz87IFxWZgVFopSCWMEg5IpmKEREHN-ySOYrVc_-J9QL_3hk8KyCiA5TlBNpTHxu90WJmwUn77bLOr5j4W5y6UF9r7SkpfnPt9Sw5GxsbLViN_IdIBk4wlrbsWFo4ZielE6y6iSp7W9grumzdxJP1_Tg";
    }
}

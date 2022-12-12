#include "AzureBlobClient.h"
#include <iostream>
#include <string>

/// Leader process that coordinates workers. Workers connect on the specified port
/// and the coordinator distributes the work of the CSV file list.
/// Example:
///    ./coordinator http://example.org/filelist.csv 4242
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <URL to csv list> <listen port>" << std::endl;
      return 1;
   }

   // TODO: add your azure credentials, get them via:
   // az storage account list
   // az account get-access-token --resource https://storage.azure.com/ -o tsv --query accessToken
   static const std::string accountName = "storageforcoursework";
   static const std::string accountToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyIsImtpZCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyJ9.eyJhdWQiOiJodHRwczovL3N0b3JhZ2UuYXp1cmUuY29tLyIsImlzcyI6Imh0dHBzOi8vc3RzLndpbmRvd3MubmV0LzVkN2I0OWU5LTUwZDItNDBkYy1iYWIxLTE0YTJkOTAzNTQyYy8iLCJpYXQiOjE2NzA4NzM0NzEsIm5iZiI6MTY3MDg3MzQ3MSwiZXhwIjoxNjcwODc4MDc1LCJhY3IiOiIxIiwiYWlvIjoiQVZRQXEvOFRBQUFBMFp3ekJPK2c5WElRMXdzWEtCMGZpYTUyWmJnRVhod2JSZTBKdWpzMjcrUkpxUVZZZnI2dlJHSjEzRCtUUXRjTXR4MjJaMDJpeTF2ckREd1BBRk9lSmhHc3hiOHZtN242RC90Sm9XS3VLTlE9IiwiYW1yIjpbInB3ZCIsIm1mYSJdLCJhcHBpZCI6IjA0YjA3Nzk1LThkZGItNDYxYS1iYmVlLTAyZjllMWJmN2I0NiIsImFwcGlkYWNyIjoiMCIsImZhbWlseV9uYW1lIjoiU2NobWlkIiwiZ2l2ZW5fbmFtZSI6IkRhbmllbCBCaW4iLCJncm91cHMiOlsiMDhlZmQ5YjItN2ExMS00NjZmLWI5NTktMThiNmMzYzQzZjNiIiwiZDAxMmU3YzctYjBiYi00NGE0LTgzMjQtZTYwZDU3ZTg1OWEyIiwiNWYxNTdhZjAtNjIzOS00OTE1LTllYmEtNDQyMGFiMTI4OTBjIl0sImlwYWRkciI6IjQ2LjEyOC4yNDUuMTM3IiwibmFtZSI6IlNjaG1pZCwgRGFuaWVsIEJpbiIsIm9pZCI6IjYyYzJjNjg2LWIxNzAtNDg3OC04ZWIzLTAwYzE5ZDhhYzA0ZSIsIm9ucHJlbV9zaWQiOiJTLTEtNS0yMS0xNDk5MjYxNzI3LTU1MTc2MTAyLTM1Mjk1MDk5MjktMTAzNjQ3NCIsInB1aWQiOiIxMDAzMjAwMjU1RDBFQkIxIiwicmgiOiIwLkFYUUE2VWw3WGRKUTNFQzZzUlNpMlFOVUxJR21CdVRVODZoQ2tMYkNzQ2xKZXZGMEFMQS4iLCJzY3AiOiJ1c2VyX2ltcGVyc29uYXRpb24iLCJzdWIiOiI0cnRhWXdrdUVZOTNTeHBZbldFMTdNVld0RVF1Z3k5SlV2bXZOVkNMbWRvIiwidGlkIjoiNWQ3YjQ5ZTktNTBkMi00MGRjLWJhYjEtMTRhMmQ5MDM1NDJjIiwidW5pcXVlX25hbWUiOiJkYW5pZWxiaW4uc2NobWlkQHR1bS5kZSIsInVwbiI6ImRhbmllbGJpbi5zY2htaWRAdHVtLmRlIiwidXRpIjoiV3RzVUlvVHlXVWloa1FMQlV1WUFBQSIsInZlciI6IjEuMCJ9.I8dltYnOooL3DOMfQrDvuqYltIHPZUmTf2aJ5gUtwh8ClRTg-HIOQKN3wzNparUeOfzKHAoLHZc_zEidRuo3vYHzZ3466ak3H9vz5O41fCb8D90DFWmRvJtJsg-lEwtFyecZ5Hyub5AolqTJlVtzD4pSfn8IblfQdw0MyuKv03fg9fTgaXUl5EihTGVz1zDfx2yLodNKs2xG-zkiJay0i3APi5hde4Ua4mlnNb-higjYiYSm-KxnaTBJDa7FgtjNvbMZLeunieJGUsHbBzsgLS6pFgQb0PW4dnQp9OzwKE7CYkVcFvNSfw5iKsUV6QSleVd3wF5wzk8ZxcR5tdueEA";
   auto blobClient = AzureBlobClient(accountName, accountToken);


   
   std::cerr << "Creating Azure blob container" << std::endl;
   // blobClient.createContainer("cbdp-assignment7");
   blobClient.setContainer("urls");
   auto blobs = blobClient.listBlobs();
   std::cout << "nBlobs: " << blobs.size() <<std::endl;
   for (const auto& blob: blobs) {
      std::cout << blob << std::endl;
   }


   auto yes = blobClient.downloadStringStream("files/filelist.csv");
   std::cout << yes.str() << std::endl;

   return 0;
}

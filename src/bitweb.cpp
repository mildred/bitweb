#if 1
#include <typeinfo>
#include <iostream>
#include "application.h"
#include "stacktrace.h"
#include "deathhandler/death_handler.h"

int main(int argc, char *argv[]){
    Debug::DeathHandler dh;
    dh.set_frames_count(100);
    //if(!stacktrace_install()) return 1;
    //try {
        bitweb::application app(argc, argv);
        if(!app.parseArguments()) return 1;
        return app.exec();
    //}
    //catch (std::exception& ex) {
    //    char *type_name = stacktrace_demangle(typeid(ex).name());
    //    std::cerr << "exception " << type_name << ": " << ex.what() << std::endl;
    //    free(type_name);
    //    stacktrace_abort();
    //}
    //catch (...) {
    //    std::cerr << "unknown exception" << std::endl;
    //    stacktrace_abort();
    //}
}
#else

#include "cryptopp/rsa.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/integer.h"
#include "cryptopp/files.h"
#include "cryptopp/osrng.h"

void DumpPrivateKey( const CryptoPP::RSAES_OAEP_SHA_Decryptor& key );
void DumpPublicKey( const CryptoPP::RSAES_OAEP_SHA_Encryptor& key );

int main(int argc, char* argv[])
{
   try {

      // Grab a Generator
      CryptoPP::AutoSeededRandomPool rng;

      // Specify modulus, accept e = 17
      CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor( rng, 64 /*, e */ );
      CryptoPP::RSAES_OAEP_SHA_Encryptor Encryptor( Decryptor );

      // BER Encoded Keys
      std::string pubkey, prvkey;

      std::cout << "=====================================" << std::endl;
      std::cout << "========= BER Encoding Keys =========" << std::endl;
      std::cout << "=====================================" << std::endl;

      CryptoPP::Base64Encoder prvencoder(
         new CryptoPP::StringSink( prvkey )
      );
      Decryptor.AccessKey().Save(prvencoder);

      std::cout << "Dumping Private Key..." << std::endl;
      DumpPrivateKey( Decryptor );
      std::cout << prvkey << std::endl;

      // BER Encode Public Key
      CryptoPP::Base64Encoder pubencoder(
         new CryptoPP::StringSink( pubkey )
      );
      Encryptor.AccessKey().Save(pubencoder);

      std::cout << "Dumping Public Key..." << std::endl;
      DumpPublicKey( Encryptor );
      std::cout << pubkey << std::endl;

      std::cout << "====================================" << std::endl;
      std::cout << "======== BER Decoding Keys =========" << std::endl;
      std::cout << "====================================" << std::endl;

      /////////////////////////////////////////////////////////
      // string should begin with 0x30 (or simply 30)
      //   0x30 is SEQUENCE_TAG.
      // The rsa400pb.dat, rsa400pv.dat, rsa1024.dat,
      //   rsa2048.dat files also use this format.
      /////////////////////////////////////////////////////////

      std::cout << prvkey << std::endl;
      //prvkey = "MIICWwIBAAKBgQCX4k8ulVBKOX6Q+8VtGzMKsquXoNMmAHuJDkABP54dm9n1SwwIQHgt2uGbW0WV2Pi5/+DSEgF0/Lw5WFxYZ80t+6afjlQMqixS3o8IJ4o06SSRIFABF/C6dWxbsr5mABMWDbn4L3Xet8z2N0Kp6UXabM8wwrEJtzNC2qq9Arhy5QIDAQABAoGAT57/y3fS41ZIl1ClKpE7rLwFYDOa/Sz7ldm2i/EBvpvZItH4uMqzNbas1nh+fhOEMF2HShjOUrm7IoTDxESGNmoL06v7y9L0UXYWvTKQ7IJPoo6cu1cUDfbWCCKSpFw9S936xPK3D45gibIgnwcrhhRQlXtZJk+XiKrJ7ApfW4ECQQDKHenaRUfob6yVYm/ycVp3sCoZZ30w3hsPChJau/EODOdYzR1YVIGqtLaCPqcJ12D+R5Z2lV6Cryp6rtXSAz1dAkEAwGAaD2LKuipNFvNIpYmk+kp9RmZAGKx4oZProW4IdiPO8mT6sQJKBkS/flXCYAYGvbuo32rZjBnOf9tN9SkrKQJAMIoMjlHTGhwvq1SPo/7bBiF1itJSu3irsOnvTIEUPBaXPLlN0dfBqDhsGE1pmjByZ77TgLkQkHv2zK3ORfhnNQJAU4+q4jIEYxcKw0RLsvP2AG4oulKh1tXb1Dt/Z3B3JnHpa2qnZgnbg5uLHmI7+x/C/9kqwlkURAHVq73jv/Dw+QJAZebIybgTlEL/8NHucyDogHwNXSEKo/dy/N9I9IsiqT37zNdjE1hazNJuINwHQ9miUkYcqGF16D4a4y0gxkYCMg==";
      //prvkey = "MIICXAIBAAKBgQDmzDn5r/5HEZHG9g/QeplxBr4hILyvblqEI3Y/X+LwsuUTB3uU2syC8Vvy8X1UyJMoKdu7cmC9Wlu9xLUECAkfAgmGfWGwAgYZf0+VFb05uTIAmeeN4D7L7dN+XbUHGiGMF/WW1ZpXQKdRNPupRR+EntylbduV2C3fJETGFaiXiQIDAQABAoGAFBLHKBRKhVvd1fQow/Uly1LOzorOXmO2s69x5Wktj8m/arxH31Z4Wxsq6CJgyDL5uZCKb+8uwslNGWA2bjdUs5dsgmBq6QRIDDv9zXqBb0Jocy3T4N7yEY2sPOk6PcS5Y7Nzvrz0WxqVCNkIS5E99WTa/rwSRfCAXLE8XWQSO1ECQQD2SyL4I4hxXmC+cj3QZMeeWw7QuF5WLQS3inmzAVCGvlzLLoXY7CDoC5Uip2/d0XIB7qBBuWNL7pZSnAhOBtrvAkEA7+TAt3ybp5o5j16fS9yWlKegH3VOq+Qf+GfNROEHpNnp6uTKLIIEF5GzLQueehl/LKGwD4Yr9LMawc1zkK0VBwJAEROkmQ/dpf6lNo81Of3C2Xf2zwH5iIAWk6NGTOMKck1AodXX8fJtVal37a8uUnLzNK8W7XetKtagFZo7+S0AgQJAIf6RjK7cw+Bn/bgT7MUilYg3eZ2++z7FhQb4oKUe1y6GJmIrgCkgeVKNSHnYCEdqrSFweJKz0eY7iXEaykqsqwJBAMdQrXSMLyQhBO8/V+T5PNiDpx13L4OnjosponefcG68oSV3TJ7kQhz6xkR+5Uc2nWxwVz2f68hZt8/s+1edtnQ=";
      //prvkey = "MIIBOgIBAAJBAK8Q+ToR4tWGshaKYRHKJ3ZmMUF6jjwCS/u1A8v1tFbQiVpBlxYBpaNcT2ENEXBGdmWqr8VwSl0NBIKyq4p0rhsCAQMCQHS1+3wL7I5ZzA8G62Exb6REINZRtCgBh/0jV91OeDnfQUc07SE6vs31J8m7qw/rxeB3E9h6oGi9IVRebVO+9zsCIQDWb//KAzrSOo0P0yktnY57UF9Q3Y26rulWI6LqpsxZDwIhAND/cmlg7rUz34PfSmM61lJEmMEjKp8RB/xgghzmCeI1AiEAjvVVMVd8jCcItTdwyRO0UjWU4JOz0cnw5BfB8cSIO18CIQCLVPbw60nOIpUClNxCJzmMLbsrbMcUtgVS6wFomVvsIwIhAK+AYqT6WwsMW2On5l9di+RPzhDT1QdGyTI5eFNS+GxY";
      //prvkey = "3053020100300D06092A864886F70D0101010500043F303D020100020900C2C58057F84BED0502011102082DD41E32677969F1020500CB71E3D1020500F51606F502050083A40BE1020500AD007D6102045B98D8DF";
      //prvkey = "MFMCAQAwDQYJKoZIhvcNAQEBBQAEPzA9AgEAAgkA1J1StSIKuE0CARECCAyBuZITu0xtAgUA6gXDywIFAOiU4UcCBQClMT7pAgRfxNU7AgUAoFBB";
      //prvkey = "MFICAQAwDQYJKoZIhvcNAQEBBQAEPjA8AgEAAgkAu0UoeVuzvHsCARECCCwQRcHMOgHRAgUA7+fs3wIFAMfVR+UCBEaPgecCBEaHgskCBQCUHVQO";
      //prvkey = "MIICXAIBAAKBgQDmzDn5r/5HEZHG9g/QeplxBr4hILyvblqEI3Y/X+LwsuUTB3uU2syC8Vvy8X1UyJMoKdu7cmC9Wlu9xLUECAkfAgmGfWGwAgYZf0+VFb05uTIAmeeN4D7L7dN+XbUHGiGMF/WW1ZpXQKdRNPupRR+EntylbduV2C3fJETGFaiXiQIDAQABAoGAFBLHKBRKhVvd1fQow/Uly1LOzorOXmO2s69x5Wktj8m/arxH31Z4Wxsq6CJgyDL5uZCKb+8uwslNGWA2bjdUs5dsgmBq6QRIDDv9zXqBb0Jocy3T4N7yEY2sPOk6PcS5Y7Nzvrz0WxqVCNkIS5E99WTa/rwSRfCAXLE8XWQSO1ECQQD2SyL4I4hxXmC+cj3QZMeeWw7QuF5WLQS3inmzAVCGvlzLLoXY7CDoC5Uip2/d0XIB7qBBuWNL7pZSnAhOBtrvAkEA7+TAt3ybp5o5j16fS9yWlKegH3VOq+Qf+GfNROEHpNnp6uTKLIIEF5GzLQueehl/LKGwD4Yr9LMawc1zkK0VBwJAEROkmQ/dpf6lNo81Of3C2Xf2zwH5iIAWk6NGTOMKck1AodXX8fJtVal37a8uUnLzNK8W7XetKtagFZo7+S0AgQJAIf6RjK7cw+Bn/bgT7MUilYg3eZ2++z7FhQb4oKUe1y6GJmIrgCkgeVKNSHnYCEdqrSFweJKz0eY7iXEaykqsqwJBAMdQrXSMLyQhBO8/V+T5PNiDpx13L4OnjosponefcG68oSV3TJ7kQhz6xkR+5Uc2nWxwVz2f68hZt8/s+1edtnQ=";
      prvkey = "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAObMOfmv/kcRkcb2D9B6mXEGviEgvK9uWoQjdj9f4vCy5RMHe5TazILxW/LxfVTIkygp27tyYL1aW73EtQQICR8CCYZ9YbACBhl/T5UVvTm5MgCZ543gPsvt035dtQcaIYwX9ZbVmldAp1E0+6lFH4Se3KVt25XYLd8kRMYVqJeJAgMBAAECgYAUEscoFEqFW93V9CjD9SXLUs7Ois5eY7azr3HlaS2Pyb9qvEffVnhbGyroImDIMvm5kIpv7y7CyU0ZYDZuN1Szl2yCYGrpBEgMO/3NeoFvQmhzLdPg3vIRjaw86To9xLljs3O+vPRbGpUI2QhLkT31ZNr+vBJF8IBcsTxdZBI7UQJBAPZLIvgjiHFeYL5yPdBkx55bDtC4XlYtBLeKebMBUIa+XMsuhdjsIOgLlSKnb93RcgHuoEG5Y0vullKcCE4G2u8CQQDv5MC3fJunmjmPXp9L3JaUp6AfdU6r5B/4Z81E4Qek2enq5MosggQXkbMtC556GX8sobAPhiv0sxrBzXOQrRUHAkARE6SZD92l/qU2jzU5/cLZd/bPAfmIgBaTo0ZM4wpyTUCh1dfx8m1VqXftry5ScvM0rxbtd60q1qAVmjv5LQCBAkAh/pGMrtzD4Gf9uBPsxSKViDd5nb77PsWFBvigpR7XLoYmYiuAKSB5Uo1IedgIR2qtIXB4krPR5juJcRrKSqyrAkEAx1CtdIwvJCEE7z9X5Pk82IOnHXcvg6eOiymid59wbryhJXdMnuRCHPrGRH7lRzadbHBXPZ/ryFm3z+z7V522dA==";

      CryptoPP::RSAES_OAEP_SHA_Encryptor temp2; // Public
      CryptoPP::RSAES_OAEP_SHA_Decryptor temp1; // Private

      { // BER Decode Private Key
         //CryptoPP::HexDecoder decoder;
         CryptoPP::Base64Decoder decoder;
         decoder.Put( (byte*)prvkey.c_str(), prvkey.size() );
         decoder.MessageEnd();

         temp1.AccessKey().Load( decoder );

         std::cout << "Dumping Private Key..." << std::endl;
         DumpPrivateKey( temp1 );
      }

      { // BER Decode Public Key
         CryptoPP::Base64Decoder decoder;
         decoder.Put( (byte*)pubkey.c_str(), pubkey.size() );
         decoder.MessageEnd();

         temp2.AccessKey().Load( decoder );

         std::cout << "Dumping Public Key..." << std::endl;
         DumpPublicKey( temp2 );
      }
   }

   catch( CryptoPP::Exception&e )
   {
      std::cerr << "Error: " << e.what() << std::endl;
   }

   catch( ... )
   {
      std::cerr << "Unknown Error" << std::endl;
   }

   return 0;
}

void DumpPrivateKey( const CryptoPP::RSAES_OAEP_SHA_Decryptor& key )
{
   std::cout << "n: " << key.GetTrapdoorFunction().GetModulus();
   std::cout << std::endl;

   std::cout << "d: " << key.GetTrapdoorFunction().GetPrivateExponent();
   std::cout << std::endl;
   std::cout << "e: " << key.GetTrapdoorFunction().GetPublicExponent();
   std::cout << std::endl;

   std::cout << "p: " << key.GetTrapdoorFunction().GetPrime1();
   std::cout << std::endl;
   std::cout << "q: " << key.GetTrapdoorFunction().GetPrime2();
   std::cout << std::endl;
}

void DumpPublicKey( const CryptoPP::RSAES_OAEP_SHA_Encryptor& key )
{
   std::cout << "n: " << key.GetTrapdoorFunction().GetModulus();
   std::cout << std::endl;

   ////////////////////////////////////////////////////////////////
   // Not in a Public Key...
   // std::cout << "d: " << key.GetTrapdoorFunction().GetPrivateExponent();
   // std::cout << std::endl;
   std::cout << "e: " << key.GetTrapdoorFunction().GetPublicExponent();
   std::cout << std::endl;

   ////////////////////////////////////////////////////////////////
   // Not in a Public Key...
   // std::cout << "p: " << key.GetTrapdoorFunction().GetPrime1();
   // std::cout << std::endl;
   // std::cout << "q: " << key.GetTrapdoorFunction().GetPrime2();
   // std::cout << std::endl;
}
#endif

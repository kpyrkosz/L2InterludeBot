#include <aes_boy.hpp>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

void
aes_boy::inplace_encrypt(char* data, unsigned size)
{
  // enc-dec mirror of what's in client
  SecByteBlock key((byte*)"\x99\x8A\x86\x3E\x91\xE5\xCD\x70\x0E\xB2\x96\x6F\xD4\x53\x6B\xEE",
                   AES::DEFAULT_KEYLENGTH);
  SecByteBlock iv((byte*)"\x50\x81\x5D\xD5\x20\x4C\x97\x97\x50\x81\x5D\xD5\x20\x4C\x97\x97",
                  AES::BLOCKSIZE);
  CFB_Mode<AES>::Encryption cfbEncryption(key, key.size(), iv);
  cfbEncryption.ProcessData((byte*)data, (byte*)data, size);
}

void
aes_boy::inplace_decrypt(char* data, unsigned size)
{
  SecByteBlock key((byte*)"\xEE\x99\x8A\x86\x3E\x91\xE5\xCD\x70\x0E\xB2\x96\x6F\xD4\x53\x6B",
                   AES::DEFAULT_KEYLENGTH);
  SecByteBlock iv((byte*)"\x50\x81\x5D\xD5\x20\x4C\x97\x97\x50\x81\x5D\xD5\x20\x4C\x97\x97",
                  AES::BLOCKSIZE);
  CFB_Mode<AES>::Decryption cfbDecryption(key, key.size(), iv);
  cfbDecryption.ProcessData((byte*)data, (byte*)data, size);
}

void
aes_boy::inplace_encrypt_to_dll(char* data, unsigned size)
{
  SecByteBlock key((byte*)"\x23\x5C\x58\x6A\x85\x2F\xE3\xD6\x5D\x10\xF0\x31\x24\x09\x61\x10",
                   AES::DEFAULT_KEYLENGTH);
  SecByteBlock iv((byte*)"\x93\x96\xA4\xF4\x44\x66\x3A\xAB\xE7\x49\x8A\x45\xD4\x3B\xEC\x73",
                  AES::BLOCKSIZE);
  CFB_Mode<AES>::Encryption cfbEncryption(key, key.size(), iv);
  cfbEncryption.ProcessData((byte*)data, (byte*)data, size);
}

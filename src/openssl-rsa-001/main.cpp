#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "openssl/err.h"
#include "openssl/pem.h"

#include "rsa_cryptor.h"

void PrintOpenSSLError() {
  std::shared_ptr<BIO> bio(BIO_new_file("/dev/stdout", "w"), BIO_free);
  if (!bio) {
    return;
  }
  ERR_print_errors(bio.get());
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <key-path-prefix>" << std::endl;
    return 1;
  }

  std::string key_path_prefix = argv[1];

  std::string public_key;
  {
    std::string path = key_path_prefix + "public.pem";
    std::ifstream ifs(path);
    if (!ifs) {
      std::cout << "Failed to load " << path << std::endl;
      return 1;
    }
    public_key.assign(std::istreambuf_iterator<char>(ifs),
                      std::istreambuf_iterator<char>());
  }
  std::string private_key;
  {
    std::string path = key_path_prefix + "private.pem";
    std::ifstream ifs(path);
    if (!ifs) {
      std::cout << "Failed to load " << path << std::endl;
      return 1;
    }
    private_key.assign(std::istreambuf_iterator<char>(ifs),
                       std::istreambuf_iterator<char>());
  }
  std::string private_key_passphrase;
  {
    std::string path = key_path_prefix + "passphrase";
    std::ifstream ifs(path);
    if (!ifs) {
      std::cout << "Failed to load " << path << std::endl;
      return 1;
    }
    std::getline(ifs, private_key_passphrase);
  }

  OpenSSL_add_all_algorithms();

  rsa::RSAPtr public_rsa;
  {
    std::shared_ptr<BIO> bio(BIO_new_mem_buf(public_key.c_str(), -1), BIO_free);
    public_rsa = rsa::Wrap(
        PEM_read_bio_RSA_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    if (!public_rsa) {
      std::cout << "Failed to parse public key." << std::endl;
      return 1;
    }
  }
  rsa::RSAPtr private_rsa;
  {
    std::shared_ptr<BIO> bio(BIO_new_mem_buf(private_key.c_str(), -1),
                             BIO_free);
    private_rsa = rsa::Wrap(PEM_read_bio_RSAPrivateKey(
        bio.get(), nullptr, nullptr,
        const_cast<char *>(private_key_passphrase.c_str())));
    if (!private_rsa) {
      std::cout << "Failed to parse private key." << std::endl;
      return 1;
    }
  }

  rsa::Cryptor public_key_cryptor(
      rsa::Key(public_rsa, rsa::Key::Type::kPublic));
  if (!public_key_cryptor.IsValid()) {
    std::cout << "public_key_cryptor is invalid: "
              << public_key_cryptor.last_error() << std::endl;
    return 1;
  }

  rsa::Cryptor private_key_cryptor(
      rsa::Key(private_rsa, rsa::Key::Type::kPrivate));
  if (!private_key_cryptor.IsValid()) {
    std::cout << "private_key_cryptor is invalid: "
              << private_key_cryptor.last_error() << std::endl;
    return 1;
  }

  std::string input;
  std::cin >> input;

  {
    std::string encrypted(public_key_cryptor.GetOutputBufferSizeForEncryption(),
                          0);
    int encrypted_size = public_key_cryptor.Encrypt(input, encrypted);
    if (encrypted_size < 0) {
      std::cout << "Failed to encrypt by public key: "
                << public_key_cryptor.last_error() << std::endl;
      PrintOpenSSLError();
      return 1;
    }
    encrypted.resize(encrypted_size);

    std::string decrypted(
        private_key_cryptor.GetOutputBufferSizeForDecryption(), 0);
    int decrypted_size = private_key_cryptor.Decrypt(encrypted, decrypted);
    if (decrypted_size < 0) {
      std::cout << "Failed to decrypt by private key: "
                << private_key_cryptor.last_error() << std::endl;
      PrintOpenSSLError();
      return 1;
    }
    decrypted.resize(decrypted_size);

    std::cout << "pub enc -> prv dec => " << decrypted << std::endl;
  }

  {
    std::string encrypted(
        private_key_cryptor.GetOutputBufferSizeForEncryption(), 0);
    int encrypted_size = private_key_cryptor.Encrypt(input, encrypted);
    if (encrypted_size < 0) {
      std::cout << "Failed to encrypt by private key: "
                << private_key_cryptor.last_error() << std::endl;
      PrintOpenSSLError();
      return 1;
    }
    encrypted.resize(encrypted_size);

    std::string decrypted(public_key_cryptor.GetOutputBufferSizeForDecryption(),
                          0);
    int decrypted_size = public_key_cryptor.Decrypt(encrypted, decrypted);
    if (decrypted_size < 0) {
      std::cout << "Failed to decrypt by public key: "
                << public_key_cryptor.last_error() << std::endl;
      PrintOpenSSLError();
      return 1;
    }
    decrypted.resize(decrypted_size);

    std::cout << "prv enc -> pub dec => " << decrypted << std::endl;
  }

  return 0;
}

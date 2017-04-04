#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/pem.h"
#include "openssl/rsa.h"

template <class Derived>
class BaseEncryptor {
 public:
  template <class Buffer>
  bool Encrypt(const Buffer &in, Buffer &out) {
    return static_cast<Derived *>(this)->Encrypt(in, out);
  }
};

template <class Derived>
class BaseDecryptor {
 public:
  template <class Buffer>
  bool Decrypt(const Buffer &in, Buffer &out) {
    return static_cast<Derived *>(this)->Decrypt(in, out);
  }
};

template <class Derived>
class BaseCryptor : public BaseEncryptor<Derived>,
                    public BaseDecryptor<Derived> {};

class RSACryptor : public BaseCryptor<RSACryptor> {
 public:
  const int kPaddingMode = RSA_PKCS1_OAEP_PADDING;
  const int kPaddingSize = RSA_PKCS1_PADDING_SIZE;
  const std::function<const EVP_MD *(void)> kDigestRoutine = EVP_sha256;

  RSACryptor(const std::shared_ptr<EVP_PKEY> &pkey) : pkey_(pkey) {}

  template <class Buffer>
  bool Encrypt(const Buffer &in, Buffer &out) {
    if (!pkey_) {
      last_error_ = "Invalid pkey.";
      return false;
    }

    // Check message size.
    // max_message_size = key_size - padding_size
    // padding_size =
    //   | mLen <= k - 2hLen - 2
    //       in OAEP by https://tools.ietf.org/html/rfc8017#section-7.1.1
    //   | 11
    //       in v1.5 by https://tools.ietf.org/html/rfc8017#section-7.2
    int max_inbuf_size;
    int key_size = RSA_size(EVP_PKEY_get1_RSA(pkey_.get()));
    const EVP_MD *md;
    if (kPaddingMode == RSA_PKCS1_OAEP_PADDING) {
      md = kDigestRoutine();
      int hash_size = EVP_MD_size(md);
      max_inbuf_size = key_size - 2 * hash_size - 2;
    } else if (kPaddingMode == RSA_PKCS1_PADDING) {
      max_inbuf_size = key_size - RSA_PKCS1_PADDING_SIZE;
    } else {
      last_error_ = "Unknown padding mode.";
      return false;
    }

    if (in.size() > max_inbuf_size) {
      last_error_ = "Too large input message size.";
      return false;
    }

    std::shared_ptr<EVP_PKEY_CTX> ctx(EVP_PKEY_CTX_new(pkey_.get(), nullptr),
                                      EVP_PKEY_CTX_free);
    if (!ctx) {
      last_error_ = "EVP_PKEY_CTX_new failed.";
      return false;
    }
    if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
      last_error_ = "EVP_PKEY_encrypt_init failed.";
      return false;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), kPaddingMode) <= 0) {
      last_error_ = "EVP_PKEY_CTX_set_rsa_padding failed.";
      return false;
    }
    if (kPaddingMode == RSA_PKCS1_OAEP_PADDING) {
      if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), md) <= 0) {
        last_error_ = "EVP_PKEY_CTX_set_rsa_oaep_md failed.";
        return false;
      }
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen,
                         reinterpret_cast<const unsigned char *>(in.data()),
                         in.size()) <= 0) {
      last_error_ = "EVP_PKEY_encrypt to obtain output buffer size failed.";
      return false;
    }
    out.resize(outlen);
    if (EVP_PKEY_encrypt(ctx.get(), reinterpret_cast<unsigned char *>(&out[0]),
                         &outlen,
                         reinterpret_cast<const unsigned char *>(in.data()),
                         in.size()) <= 0) {
      last_error_ = "EVP_PKEY_encrypt to encrypt input buffer size failed.";
      return false;
    }
    out.resize(outlen);

    return true;
  }

  template <class Buffer>
  bool Decrypt(const Buffer &in, Buffer &out) {
    if (!pkey_) {
      return false;
    }

    // Check message size.
    int max_inbuf_size = RSA_size(EVP_PKEY_get1_RSA(pkey_.get()));
    if (in.size() > max_inbuf_size) {
      return false;
    }

    std::shared_ptr<EVP_PKEY_CTX> ctx(EVP_PKEY_CTX_new(pkey_.get(), nullptr),
                                      EVP_PKEY_CTX_free);
    if (!ctx) {
      return false;
    }
    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) {
      return false;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), kPaddingMode) <= 0) {
      return false;
    }
    if (kPaddingMode == RSA_PKCS1_OAEP_PADDING) {
      if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), kDigestRoutine()) <= 0) {
        last_error_ = "EVP_PKEY_CTX_set_rsa_oaep_md failed.";
        return false;
      }
    }

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen,
                         reinterpret_cast<const unsigned char *>(in.data()),
                         in.size()) <= 0) {
      return false;
    }
    out.resize(outlen);
    if (EVP_PKEY_decrypt(ctx.get(), reinterpret_cast<unsigned char *>(&out[0]),
                         &outlen,
                         reinterpret_cast<const unsigned char *>(in.data()),
                         in.size()) <= 0) {
      return false;
    }
    out.resize(outlen);

    return true;
  }

  const std::string &last_error() { return last_error_; }

 private:
  std::shared_ptr<EVP_PKEY> pkey_;
  std::string last_error_;
};

inline void PrintOpenSSLError() {
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

  std::shared_ptr<EVP_PKEY> public_pkey(nullptr, EVP_PKEY_free);
  {
    std::shared_ptr<BIO> bio(BIO_new_mem_buf(public_key.c_str(), -1), BIO_free);
    public_pkey.reset(
        PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
    if (!public_pkey) {
      std::cout << "Failed to parse public key." << std::endl;
      return 1;
    }
  }
  std::shared_ptr<EVP_PKEY> private_pkey(nullptr, EVP_PKEY_free);
  {
    std::shared_ptr<BIO> bio(BIO_new_mem_buf(private_key.c_str(), -1),
                             BIO_free);
    private_pkey.reset(PEM_read_bio_PrivateKey(
        bio.get(), nullptr, nullptr,
        const_cast<char *>(private_key_passphrase.c_str())));
    if (!private_pkey) {
      std::cout << "Failed to parse private key." << std::endl;
      return 1;
    }
  }

  RSACryptor public_key_cryptor(public_pkey);
  RSACryptor private_key_cryptor(private_pkey);

  std::string input;
  std::cin >> input;

  {
    std::string encrypted, decrypted;
    if (!public_key_cryptor.Encrypt(input, encrypted)) {
      std::cout << "Failed to encrypt by public key: "
                << public_key_cryptor.last_error() << std::endl;
      PrintOpenSSLError();
      return 1;
    }
    if (!private_key_cryptor.Decrypt(encrypted, decrypted)) {
      std::cout << "Failed to decrypt by private key:"
                << public_key_cryptor.last_error() << std::endl;
      return 1;
    }
    std::cout << "pub enc -> prv dec => " << decrypted << std::endl;
  }

  {
    std::string encrypted, decrypted;
    if (!private_key_cryptor.Encrypt(input, encrypted)) {
      std::cout << "Failed to encrypt by private key:"
                << public_key_cryptor.last_error() << std::endl;
      return 1;
    }
    if (!public_key_cryptor.Decrypt(encrypted, decrypted)) {
      std::cout << "Failed to decrypt by public key:"
                << public_key_cryptor.last_error() << std::endl;
      return 1;
    }
    std::cout << "prv enc -> pub dec => " << decrypted << std::endl;
  }

  return 0;
}

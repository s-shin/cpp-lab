#ifndef RSA_CRYPTOR_H_
#define RSA_CRYPTOR_H_

#include <memory>
#include <string>

#include "openssl/evp.h"
#include "openssl/rsa.h"

namespace rsa {

template <class T>
inline typename T::size_type container_sizeof(T v) {
  return sizeof(typename T::value_type) * v.size();
}

using RSAPtr = std::shared_ptr<RSA>;

inline RSAPtr Wrap(RSA *rsa) {
  std::shared_ptr<RSA> p(rsa, RSA_free);
  return p;
}

enum class PaddingMode {
  // OAEP is unavailable for both encryption by private key and decryption by
  // public key because OpenSSL is not supported.
  // In the background, encryption by private key and decryption by public key
  // are only used for not encryption but signing.
  kOAEP = RSA_PKCS1_OAEP_PADDING,
  kV1_5 = RSA_PKCS1_PADDING,
};

struct CryptoOption {
  PaddingMode padding_mode;
  const EVP_MD *md;
};

constexpr CryptoOption DefaultCryptoOption() {
  return {PaddingMode::kV1_5, nullptr};
}

int GetPaddingSize(const CryptoOption &opt) {
  switch (opt.padding_mode) {
    case PaddingMode::kOAEP: {
      // https://tools.ietf.org/html/rfc8017#section-7.1.1
      const EVP_MD *md = opt.md;
      if (md == nullptr) {
        md = EVP_sha1();
      }
      int md_size = EVP_MD_size(md);
      return md_size * 2 + 2;
    }
    case PaddingMode::kV1_5: {
      // https://tools.ietf.org/html/rfc8017#section-7.2
      return RSA_PKCS1_PADDING_SIZE;
    }
  }
  return -1;
}

class Key {
 public:
  enum class Type { kPublic, kPrivate };

  Key(RSAPtr rsa, Type type) : rsa_(rsa), type_(type) {}

  int size() const { return RSA_size(rsa_.get()); }
  Type type() const { return type_; }

  explicit operator bool() const { return !!rsa_; }
  operator RSA *() const { return rsa_.get(); }

 private:
  RSAPtr rsa_;
  Type type_;
};

class Cryptor {
 public:
  Cryptor(Key key) : Cryptor(std::move(key), DefaultCryptoOption()) {}

  Cryptor(Key key, CryptoOption option)
      : key_(std::move(key)), option_(std::move(option)) {}

  bool IsValid() {
    if (!key_) {
      last_error_ = "Invalid key.";
      return false;
    }
    return true;
  }

  int GetMaxEncryptableBufferSize() {
    if (!key_) {
      last_error_ = "Invalid key.";
      return -1;
    }
    return key_.size() - GetPaddingSize(option_);
  }

  int GetMaxDecryptableBufferSize() {
    if (!key_) {
      last_error_ = "Invalid key.";
      return -1;
    }
    return key_.size();
  }

  int GetOutputBufferSizeForEncryption() { return key_.size(); }

  int GetOutputBufferSizeForDecryption() { return key_.size(); }

  template <class SrcBuffer, class DstBuffer>
  int Encrypt(const SrcBuffer &src, DstBuffer &dst) {
    return Encrypt(
        reinterpret_cast<const unsigned char *>(&src[0]), container_sizeof(src),
        reinterpret_cast<unsigned char *>(&dst[0]), container_sizeof(dst));
  }

  int Encrypt(const unsigned char *src_buf, int src_buf_size,
              unsigned char *dst_buf, int dst_buf_size) {
    if (!IsValid()) {
      return -1;
    }
    if (key_.type() == Key::Type::kPrivate &&
        option_.padding_mode == PaddingMode::kOAEP) {
      last_error_ = "OAEP cannot be used for encyption by private key.";
      return -1;
    }

    if (src_buf_size > GetMaxEncryptableBufferSize()) {
      last_error_ = "Too large input buffer size.";
      return -1;
    }
    if (dst_buf_size < GetOutputBufferSizeForEncryption()) {
      last_error_ = "Too small output buffer size.";
      return -1;
    }
    int encrypted_size;
    if (key_.type() == Key::Type::kPrivate) {
      encrypted_size =
          RSA_private_encrypt(src_buf_size, src_buf, dst_buf, key_,
                              static_cast<int>(option_.padding_mode));
    } else {
      encrypted_size =
          RSA_public_encrypt(src_buf_size, src_buf, dst_buf, key_,
                             static_cast<int>(option_.padding_mode));
    }
    if (encrypted_size == -1) {
      last_error_ = "Failed to encrypt.";
      return -1;
    }
    return encrypted_size;
  }

  template <class SrcBuffer, class DstBuffer>
  int Decrypt(const SrcBuffer &src, DstBuffer &dst) {
    return Decrypt(
        reinterpret_cast<const unsigned char *>(&src[0]), container_sizeof(src),
        reinterpret_cast<unsigned char *>(&dst[0]), container_sizeof(dst));
  }

  int Decrypt(const unsigned char *src_buf, int src_buf_size,
              unsigned char *dst_buf, int dst_buf_size) {
    if (!IsValid()) {
      return -1;
    }
    if (key_.type() == Key::Type::kPublic &&
        option_.padding_mode == PaddingMode::kOAEP) {
      last_error_ = "OAEP cannot be used for decyption by public key.";
      return -1;
    }

    if (src_buf_size > GetMaxDecryptableBufferSize()) {
      last_error_ = "Too large input buffer size.";
      return -1;
    }
    if (dst_buf_size < GetOutputBufferSizeForDecryption()) {
      last_error_ = "Too small output buffer size.";
      return -1;
    }
    int decrypted_size;
    if (key_.type() == Key::Type::kPrivate) {
      decrypted_size =
          RSA_private_decrypt(src_buf_size, src_buf, dst_buf, key_,
                              static_cast<int>(option_.padding_mode));
    } else {
      decrypted_size =
          RSA_public_decrypt(src_buf_size, src_buf, dst_buf, key_,
                             static_cast<int>(option_.padding_mode));
    }
    if (decrypted_size == -1) {
      last_error_ = "Failed to decrypt.";
      return -1;
    }
    return decrypted_size;
  }

  const std::string &last_error() const { return last_error_; }

 private:
  Key key_;
  CryptoOption option_;
  std::string last_error_;
};

}  // namespace rsa

#endif  // RSA_CRYPTOR_H_

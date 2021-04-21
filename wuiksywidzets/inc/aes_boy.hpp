#pragma once

namespace aes_boy
{
void inplace_encrypt(char* data, unsigned size);
void inplace_decrypt(char* data, unsigned size);

void inplace_encrypt_to_dll(char* data, unsigned size);
} // namespace aes_boy

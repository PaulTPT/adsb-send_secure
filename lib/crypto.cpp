//
// Created by paultpt on 21/06/16.
//

#include "crypto.h"


//generate a bytewise lookup CRC table

unsigned int crc_table[256];

void generate_crc_table(void)
{
    unsigned int crc = 0;
    for(int n=0; n<256; n++) {
        crc = n<<16;
        for(int k=0; k<8; k++) {
            if(crc & 0x800000) {
                crc = ((crc<<1) ^ POLY) & 0xFFFFFF;
            } else {
                crc = (crc<<1) & 0xFFFFFF;
            }
        }
        crc_table[n] = crc & 0xFFFFFF;
    }
}

//Perform a bytewise CRC check
uint32_t modesChecksum(unsigned char *msg, int bytes, unsigned char r_crc[3])
{
    if(crc_table[1] != POLY) generate_crc_table();
    unsigned int crc=0;
    for(int i=0; i<bytes; i++) {
        crc = crc_table[((crc>>16) ^ msg[i]) & 0xff] ^ (crc << 8);
    }
    crc &= 0xFFFFFF;

    memcpy(r_crc,&crc,3*sizeof(char));
    unsigned char t_crc =r_crc[2];
    r_crc[2]=r_crc[0];
    r_crc[0]=t_crc;

    return crc; /* 24 bit checksum. */

}

void Crypto::gen_signature(const unsigned char addr[3], const unsigned char sync, const unsigned char key[32], unsigned char sig[39+SIGLENGTH]) {

    unsigned char gen_sig[1+35+3+SIGLENGTH];
    unsigned char input[35];
    memcpy(input,addr,3*sizeof(char));
    memcpy(input,key,32*sizeof(char));

    EC_KEY *eckey = NULL;
    EC_POINT *pub_key = NULL;
    const EC_GROUP *group = NULL;
    BIGNUM start;
    BIGNUM *res;
    BN_CTX *ctx;

    BN_init(&start);
    ctx = BN_CTX_new(); // ctx is an optional buffer to save time from allocating and dealocating memory whenever required
    res = &start;
    BN_hex2bn(&res,"267B419FDC0E6929FF1127D27AB4E09C"); //Private key of the CA
    eckey = EC_KEY_new_by_curve_name(NID_secp128r1);
    group = EC_KEY_get0_group(eckey);
    pub_key = EC_POINT_new(group);
    EC_KEY_set_private_key(eckey, res);


    /* pub_key is a new uninitialized `EC_POINT*`.  priv_key res is a `BIGNUM*`. */
    if (!EC_POINT_mul(group, pub_key, res, NULL, NULL, ctx))
        printf("Error at EC_POINT_mul.\n");
    EC_KEY_set_public_key(eckey, pub_key);

    ECDSA_SIG *signature = ECDSA_do_sign(input, 36, eckey);
    if (NULL == signature)
        printf("Failed to generate EC Signature\n");

    EC_KEY_free(eckey);
    BN_CTX_free(ctx);


    int sigSize = i2d_ECDSA_SIG(signature, NULL);
    unsigned  char* derSig = (unsigned char*)malloc(sigSize);
    unsigned char* p = derSig;    //memset(sig_bytes, 6, sig_size);
    sigSize= i2d_ECDSA_SIG(signature, &p);

    if (sigSize/8>SIGLENGTH)
        printf("Error: signature is too long. Choose an other curve");

    unsigned char header=0b01110000 | (sync & 0b00000111); //Type 14

    unsigned char crc[3];

    memcpy(gen_sig,&header,1*sizeof(char));
    memcpy(gen_sig+1,input,35*sizeof(char));
    memcpy(gen_sig+36,derSig,SIGLENGTH*sizeof(char));
    modesChecksum(gen_sig,36+SIGLENGTH,crc);
    memcpy(gen_sig+36+SIGLENGTH,crc,3*sizeof(char));
    memcpy(sig,gen_sig,(39+SIGLENGTH)*sizeof(char));


}

void Crypto::publish_key(const unsigned char addr[3], const unsigned char key[32], unsigned char key_mess[39]){

    unsigned char crc[3];

    unsigned char header=0b01111000; //Type 15
    memcpy(key_mess,&header,1*sizeof(char));
    memcpy(key_mess+1,addr,3*sizeof(char));
    memcpy(key_mess+4,key,32*sizeof(char));
    modesChecksum(key_mess,36,crc);
    memcpy(key_mess+36,crc,3*sizeof(char));

}

void Crypto::gen_keys(const unsigned char root_key[32], unsigned char keys[NUMKEYS][32], int nb_keys, unsigned char last_key[32]){

    unsigned char digest[32];
    unsigned char hash_key[32]={0x88, 0x08, 0x21, 0x26, 0xb1, 0x08, 0xc1, 0xc1, 0x00, 0xd4, 0x58, 0xc3, 0xa8,\
     0x25, 0x63, 0x36, 0xde, 0xb2, 0x32, 0x88, 0xf6, 0x95, 0x4e, 0xa7, 0x4e, 0xf1, 0x3c, 0x33, 0x4e, 0x70, 0x50, 0x8f};
    unsigned char last_keyl[32];

    memcpy(last_keyl,root_key,32*sizeof(char));
    memcpy(keys[nb_keys],last_keyl,32*sizeof(char));

    for (int i = nb_keys-1; i>=0;i--) {
        memcpy(last_keyl, HMAC(EVP_sha256(),hash_key, 32, last_keyl, 32, NULL, NULL),32*sizeof(char));
        memcpy(keys[i],last_keyl,32*sizeof(char));
    }

    memcpy(last_key,last_keyl,32*sizeof(char));
}
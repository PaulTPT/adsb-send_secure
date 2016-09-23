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
    memcpy(input+3,key,32*sizeof(char));

    FILE *fp;

    fp = fopen ("/home/paultpt/Documents/eckeys/ec_pkey.pem", "rb");
        if (fp==NULL){
        printf("Reading key FAIL\n");
     }
     else
        printf("Readig key SUCCESS\n");

     EVP_PKEY *privkey;
     privkey = PEM_read_PrivateKey( fp, NULL,0, NULL);
     if (privkey==NULL){
        printf("Loading key FAIL\n");
     }
     else
        printf("Loading key SUCCESS\n");
    fclose(fp);

    EC_KEY *eckey;
    eckey = EC_KEY_new();
    eckey = EVP_PKEY_get1_EC_KEY(privkey);

    if (!EC_KEY_check_key(eckey))
    {
        printf("Error with ECKey.\n");
    }


    unsigned char header=0b01110000 | (sync & 0b00000111); //Type 14



    memcpy(gen_sig,&header,1*sizeof(char));
    memcpy(gen_sig+1,input,35*sizeof(char));

    //Signature

    unsigned char hash[20];
    SHA1(gen_sig, 36, hash);

    std::cout << "Sha : ";
    for (int m=0; m<20; m++){
        std::cout <<  std::hex << std::setw(2) << std::setfill('0') << unsigned(hash[m]);
    }
    std::cout <<std::endl;

    ECDSA_SIG * ecdsa_sig;
    ecdsa_sig = ECDSA_SIG_new();
    unsigned int siglen = ECDSA_size(eckey);


    ecdsa_sig = ECDSA_do_sign(hash, 20, eckey);
    if (NULL == ecdsa_sig)
        printf("Failed to generate EC Signature\n");



    //We use the NID_secp128r1 signature, then r and s are 128 bits long (16 bytes)
    BIGNUM* r = ecdsa_sig->r;
    int r_size=BN_num_bytes(r);
    unsigned char r_bytes[r_size];
    BN_bn2bin(r,r_bytes);

    std::cout << "r : ";
    for (int m=0; m<r_size; m++){
        std::cout <<  std::hex << std::setw(2) << std::setfill('0') << unsigned(r_bytes[m]);
    }
    std::cout <<std::endl;

    BIGNUM* s =ecdsa_sig->s;
    int s_size=BN_num_bytes(s);
    unsigned char s_bytes[s_size];
    BN_bn2bin(s,s_bytes);

    std::cout << "s : ";
    for (int m=0; m<s_size; m++){
        std::cout <<  std::hex << std::setw(2) << std::setfill('0') << unsigned(s_bytes[m]);
    }
    std::cout <<std::endl;

    if (r_size+s_size>SIGLENGTH){
        printf("Signature too long ...\n");
    }

    EC_KEY_free(eckey);

    memcpy(gen_sig+36,r_bytes,r_size*sizeof(char));
    memcpy(gen_sig+36+r_size,s_bytes,s_size*sizeof(char));
    //CRC
    unsigned char crc[3];
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
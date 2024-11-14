#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <gmp.h>
#include <string.h>
#include <png.h>

// 1 - bullshit
// 2 - useful output
// 3 - errors
unsigned short PRIO_write_log = 2;

// ключі rsa
struct rsa_keys
{
    mpz_t public; // e
    mpz_t private; // d
    mpz_t N;
};

// глобальні змінні необхідні для роботи із зображеннями
int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

// функція отримання ключів
struct rsa_keys get_rsa_keys(int complexity);
// Функція перетворює text в число та шифрує за допомогою rsa, результат записусє в cypher_text
void encrypt_rsa(mpz_t* cypher_text, struct rsa_keys k, char* text);
// Функція розшифровує cypher_text і перетворює в текст, результат записусє в buf
void decrypt_rsa(char * buf, struct rsa_keys k, mpz_t* cypher_text);

void write_log(int priority, const char *format, ...);
int get_rand(int down, int up);
// перевіряє чи просте число є сильним, тобто більшим за півсуму попереднього і наступного
bool isStrong(mpz_t p);

void str_to_num(mpz_t* num, const unsigned char *str);
void num_to_str(char (*buf), mpz_t num);

// Робота з зображенням
void read_png_file(char *filename);
void write_png_file(char *filename);
void write_code_in_png_file(char *str, int len);
void read_code(char *str);


int main() {
    struct rsa_keys k = get_rsa_keys(6000);
    gmp_printf("Got Keys:\n\te(public exponent): %Zd\n\td(private exponent): %Zd\n\tN: %Zd\n", k.public, k.private, k.N);

    mpz_t msg;
    mpz_init(msg);

    encrypt_rsa(&msg, k, "Hi, моє сонце!");
    gmp_printf("Encrypted: %Zd\n", msg);
    read_png_file("./image.png");
    char binBuf[mpz_sizeinbase(msg, 2)+1];
    char binMsg[mpz_sizeinbase(msg, 2)+23];
    mpz_get_str(binBuf, 2, msg);
    strcpy(binMsg, binBuf);
    strcat(binMsg, "000000000000011111111");
    gmp_printf("\tTurned rsa num into binary and added 000000000000011111111 to the end: %s\n", binMsg);
    write_code_in_png_file(binMsg, mpz_sizeinbase(msg, 2)+24);
    write_png_file("./img.png");
    
    // DECRYPTION
    
    read_png_file("./img.png");
    // gmp_printf("BIN: %s\n", binMsg);
    char binBuf2[1000];
    read_code(binBuf2);
    mpz_set_str(msg, binBuf2, 2);

    char buf[100];
    decrypt_rsa(buf, k, msg);
    
    printf("! Original = %s\n", buf);

    return 0;
}

struct rsa_keys get_rsa_keys(int complexity) {
    srand(time(NULL));
    int limit = 10000 * complexity;
    
    struct rsa_keys keys;
    mpz_t p, q, phi, N, d, e, g, s, t;
    mpz_init(p);
    mpz_init(q);
    mpz_init(phi);
    mpz_init(N);
    mpz_init_set_ui(d, 1L);
    mpz_init(e);
    mpz_inits(g, s, t);

    mpz_set_str(p, "3948230942834092834092834029384230948293048", 10);
    mpz_set_str(q, "234987234982719823719823795872398742987928347", 10);

    do {
        mpz_nextprime(p, p);
    } while( mpz_probab_prime_p(p, 1) == 0 || !isStrong(p) );
    do {
        mpz_nextprime(q, q);
    } while( mpz_probab_prime_p(p, 1) == 0  || !isStrong(q) );

    mpz_mul(N, p, q);
    mpz_sub_ui(p, p, 1L);
    mpz_sub_ui(q, q, 1L);
    mpz_mul(phi, p, q);
    mpz_set_ui(e, 65579);

    mpz_gcdext(g, s, t, e, phi);

    mpz_mod(d, s, phi);

    mpz_init_set(keys.N, N);
    mpz_init_set(keys.private, d);
    mpz_init_set(keys.public, e);

    return keys;
}

void encrypt_rsa(mpz_t* cypher_text, struct rsa_keys k, char* text) {
    // mpz_t e, N;
    // mpz_init_set(e, k.public);
    // mpz_init_set(N, k.N);

    mpz_init(cypher_text);
    // char txt = 'I love you';
    // char* fmt[22], buf[100];
    // for (int i <= )
    char buf[1000];
    sprintf(buf, "%s%s", "Ш", text);
    str_to_num(cypher_text, buf);
    gmp_printf("\tencrypt_rsa: Turned text %s to number using unicode codes: %Zd\n", buf, cypher_text);
    // mpz_set(msg, *str_to_num("Солік, я тебе лублу"));
    mpz_powm(cypher_text, cypher_text, k.public, k.N);

    // gmp_printf("Encrypted = %Zd\n", cypher_text);
}

void decrypt_rsa(char * buf, struct rsa_keys k, mpz_t* cypher_text) {
    mpz_powm(cypher_text, cypher_text, k.private, k.N);
    gmp_printf("Decrypted: %Zd\n", cypher_text);
    
    num_to_str(buf, cypher_text);
}

void write_log(int priority, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if(priority >= PRIO_write_log) {
        // printf("|LOG %d| ", priority);
        vprintf(format, args);
    }

    va_end(args);
}

int get_rand(int down, int up) {
    int r = down + (rand() % (up - down + 1));
    return r;
}

bool isStrong(mpz_t p) {
    mpz_t prev, next;
    mpz_inits(next, prev, NULL);
    mpz_prevprime(prev, p);
    mpz_nextprime(next, p);

    mpz_add(next, next, prev);
    mpz_div_ui(next, next, 2);
    return mpz_cmp(p, next) == 1;
}

void str_to_num(mpz_t* num, const unsigned char *str)
{
    // mpz_t num;
    // mpz_init(num);

    // const char *str = "Солік, я тебе лублу"; // Example Cyrillic string

    const unsigned char *p = (const unsigned char *)str;
    // printf("Hello %s", p);

    int n = 0;

    while (*p) {
        unsigned int codepoint = 0;

        if ((*p & 0x80) == 0) {
            // Single byte (ASCII character)
            codepoint = *p;
            // printf("0 Character: %c, Code: %u\n", *p, codepoint);
            p++;
            n++;
        } else if ((*p & 0xE0) == 0xC0) {
            // Two-byte UTF-8 character
            codepoint = ((*p & 0x1F) << 6) | (*(p + 1) & 0x3F);
            // printf("1 Character: %c%c, Code: %u\n", *p, *(p + 1), codepoint);
            p += 2;
            n += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            // Three-byte UTF-8 character (common for Cyrillic)
            codepoint = ((*p & 0x0F) << 12) | ((*(p + 1) & 0x3F) << 6) | (*(p + 2) & 0x3F);
            // printf("2 Character: %c%c%c, Code: %u\n", *p, *(p + 1), *(p + 2), codepoint);
            p += 3;
            n += 3;
        } 
        else if ((*p & 0xF8) == 0xF0) {
            // Three-byte UTF-8 character (common for Cyrillic)
            codepoint = ((*p & 0x07) << 18) | ((*(p + 1) & 0x3F) << 12) | ((*(p + 2) & 0x3F) << 6) | (*(p + 3) & 0x3F);
            // printf("4 Character: %c%c%c%c, Code: %u\n", *p, *(p + 1), *(p + 2), *(p + 3), codepoint);
            p += 4;
            n += 4;
        } 
        else {
            // Handle any other encoding scenario if needed
            p++;
            n++;
        }
        mpz_mul_ui(num, num, 10000L);
        mpz_add_ui(num, num, codepoint);
        // gmp_printf("!! NUmber: %Zd %d\n", num, n);
        // printf("NUmber: %ld\n", num);
    }
}

void num_to_str(char *buf, mpz_t num)
{
    int n = (int)mpz_sizeinbase(num, 10);
    int buffsize = 0;
    char* digits;
    gmp_sprintf(digits, "%Zd", num);
    for (int i = 4; i < n; i+=4)
    {
        char str_code[] = {digits[i], digits[i+1], digits[i+2], digits[i+3]};
        int code = atoi(str_code);
        if (code < 1000) {
            buffsize +=1;
        } else {
            buffsize += 2;
        }
    }
    char new_buf[buffsize+1];
    
    mpz_t code;
    mpz_init(code);
    
    int buffsize_fiexed = buffsize;

    while(mpz_cmp_ui(num, 0)) {
        mpz_mod_ui(code, num, 10000);
        mpz_div_ui(num, num, 10000);
        if (mpz_get_ui(code) <= 0x7F) {
            // 1-byte UTF-8 (ASCII)
            new_buf[buffsize-1] = mpz_get_ui(code);
            buffsize--;
        } else if (mpz_get_ui(code) <= 0x7FF) {
            // 2-byte UTF-8
            new_buf[buffsize-2] = 0xC0 | (mpz_get_ui(code) >> 6);         // Upper 5 bits
            new_buf[buffsize-1] = 0x80 | (mpz_get_ui(code) & 0x3F);       // Lower 6 bits
            buffsize-=2;
        } else if (mpz_get_ui(code) <= 0xFFFF) {
            // 3-byte UTF-8
            new_buf[buffsize-3] = 0xE0 | (mpz_get_ui(code) >> 12);        // Upper 4 bits
            new_buf[buffsize-2] = 0x80 | ((mpz_get_ui(code) >> 6) & 0x3F); // Next 6 bits
            new_buf[buffsize-1] = 0x80 | (mpz_get_ui(code) & 0x3F);       // Lower 6 bits
        } 
    }

    for (size_t i = 0; i <= buffsize_fiexed; i++)
    {
        buf[i] = new_buf[i]; 
    }
    buf[buffsize_fiexed] = ' ';
    buf[buffsize_fiexed+1] = ' ';
    buf[buffsize_fiexed+2] = ' ';
    buf[buffsize_fiexed+3] = ' ';
}

void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");
  if(!fp) abort();
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();
  png_infop info = png_create_info_struct(png);
  if(!info) abort();
  if(setjmp(png_jmpbuf(png))) abort();
  png_init_io(png, fp);
  png_read_info(png, info);
  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
}

void write_png_file(char *filename) {
  int y;

  FILE *fp = fopen(filename, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );

  png_set_expand(png);
  png_write_info(png, info);
  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  fclose(fp);
}

void write_code_in_png_file(char *str, int len) {
    for (int i = 0; i < len; i++)
    {
        png_bytep row = row_pointers[i];
        png_bytep px = &(row[i * 4]);
        if (str[i] == 49) {
            if ((int)px[0] % 2 == 0) {
                px[0] = (int)px[0] + 1;
            }
        } else {
            if ((int)px[0] % 2 != 0) {
                px[0] = (int)px[0] + 1;
            }
        }
    }
}

void read_code(char* str) {

    int size = 0;

    for (int i = 0; i < height; i++)
    {
        png_bytep row = row_pointers[i];
        png_bytep px = &(row[i * 4]);

        if ((int)px[0] % 2 == 0) {
            str[i] = '0';
        } else {
            str[i] = '1';
        }
        size++;
        if (strstr(str, "000000000000011111111") != NULL) {
            break;
        }
    }
    
    str[size-21] = '\0';
}

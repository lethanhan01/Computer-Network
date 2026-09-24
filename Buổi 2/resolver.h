#ifndef RESOLVER_H
#define RESOLVER_H

/**
 * @file resolver.h
 * @brief Khai bao module phan giai ten mien sang dia chi IP.
 */

#define MAX_ALIASES 64
#define IPV4_STRING_BUFFER_SIZE 16

/**
 * @brief Phan giai ten mien sang dia chi IP chinh thuc va danh sach IP phu.
 *
 * @param[in] domainName Ten mien can phan giai.
 * @param[out] officialIp Buffer luu dia chi IP chinh thuc dau tien tim thay.
 * @param[out] aliasIps Mang 2 chieu chua danh sach cac IP phu tiep theo.
 * @param[in] maxAliases So luong IP phu toi da co the luu vao mang.
 * @param[out] aliasCount Con tro luu so luong IP phu thuc te tim thay.
 *
 * @return `1` neu phan giai thanh cong it nhat mot dia chi IP, `0` neu that bai.
 */
int resolveDomain(const char* domainName, char* officialIp,
                  char (*aliasIps)[IPV4_STRING_BUFFER_SIZE],
                  int maxAliases, int* aliasCount);

/**
 * @brief In ket qua phan giai ten mien theo dung dinh dang yeu cau cua de bai.
 *
 * @param[in] officialIp Dia chi IP chinh thuc.
 * @param[in] aliasIps Mang cac dia chi IP phu.
 * @param[in] aliasCount So luong IP phu can in.
 */
void printDomainResult(const char* officialIp,
                       char (*aliasIps)[IPV4_STRING_BUFFER_SIZE],
                       int aliasCount);

#endif /* RESOLVER_H */

#ifndef P2P_TECHDEMO_CORE_IP_ADDRESS_H
#define P2P_TECHDEMO_CORE_IP_ADDRESS_H

#include <array>

struct IpAddress
{
private:
    union
    {
        uint8_t _field8[16];
        uint16_t _field16[8];
        uint32_t _field32[4];
    };

    bool _valid;

    bool _wildcard;

private:
    void _parse_ipv4(const std::string &str, int start, uint8_t *ret);

    void _parse_ipv6(const std::string &str);

public:
    bool operator!=(const IpAddress &ip_address) const;

    bool operator==(const IpAddress &ip_address) const;

    void clear();

    explicit operator std::string() const;

    bool is_ipv4() const;

    bool is_valid() const;

    bool is_wildcard() const;

    const uint8_t *GetIPv4() const;

    const uint8_t *GetIPv6() const;

    void set_ipv4(const uint8_t (&ip)[4]);

    void set_ipv6(const uint8_t (&ip)[16]);
    void set_ipv6(const std::array<uint8_t, 16> &host);

    explicit IpAddress(const std::string &str);

    IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6 = false);

    IpAddress();
};

#endif // P2P_TECHDEMO_CORE_IP_ADDRESS_H

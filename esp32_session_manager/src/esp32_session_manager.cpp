/*
 Name:		esp32_session_manager.cpp
 Created:	1/20/2023 10:48:51 PM
 Author:	jiaji
 Editor:	http://www.visualmicro.com
*/

#include "esp32_session_manager.h"

JSONVar session_info_t::toJSON()
{
    JSONVar session_JSON;
    session_JSON["ip"] = _ip;
    session_JSON["conn"] = static_cast<uint8_t>(_conn);
    session_JSON["id"] = _userID;
    for (uint8_t i = 0, *it = std::begin(_mac.addr); it != std::end(_mac.addr); ++it, ++i)
        session_JSON["mac"][i] = *it;
    return session_JSON;
}

session_info_t::session_info_t() {}

session_info_t::session_info_t(const uint32_t& ip, const eth_addr& mac, const wifi_interface_t& conn, const String& id) : _ip(ip), _mac(mac), _conn(conn), _userID(id)
{
    log_d("Session created");
    log_d("IP: %u", _ip);
    log_d("MAC: %u:%u:%u:%u:%u:%u", _mac.addr[0], _mac.addr[1], _mac.addr[2], _mac.addr[3], _mac.addr[4], _mac.addr[5], _mac.addr[6]);
    log_d("CONN: %i", _conn);
    log_d("ID: %s", _userID);
}

session_info_t::~session_info_t()
{
    log_d("Session removed:");
    log_d("IP: %u", _ip);
    log_d("MAC: %u:%u:%u:%u:%u:%u", _mac.addr[0], _mac.addr[1], _mac.addr[2], _mac.addr[3], _mac.addr[4], _mac.addr[5], _mac.addr[6]);
    log_d("ID: %s", _userID);
}

session_info_t::operator bool() const
{
    return _userID.length();
}

namespace cst
{
    eth_addr* ESPSessionManager::get_sta_mac(const uint32_t& ip)
    {
        ip4_addr requestIP{ ip };
        eth_addr* ret_eth_addr = nullptr;
        ip4_addr const* ret_ip_addr = nullptr;
        etharp_request(netif_default, &requestIP);
        etharp_find_addr(netif_default, &requestIP, &ret_eth_addr, &ret_ip_addr);
        return ret_eth_addr;
    }

    const session_info_t& ESPSessionManager::handle_ap_ip(const uint32_t& ip) // TODO: rewrite using std::optional
    {
        auto thisSession = ap_sessions.find(ip);
        return (thisSession != ap_sessions.end()) ? thisSession->second : emptySession;
    }

    const session_info_t& ESPSessionManager::handle_sta_ip(const uint32_t& ip) // TODO: rewrite using std::optional
    {
        auto thisSession = sta_sessions.find(ip);
        if (thisSession != sta_sessions.end())
        {
            if (std::equal(std::begin(thisSession->second._mac.addr), std::end(thisSession->second._mac.addr), std::begin(get_sta_mac(ip)->addr)))
                return thisSession->second;
            sta_sessions.erase(ip);
        }
        return emptySession;
    }

    const session_info_t& ESPSessionManager::get_session_info(const IPAddress& localIP, const uint32_t& remoteAddr) // TODO: rewrite using std::optional
    {
        if (is_ap(localIP))
            return handle_ap_ip(remoteAddr);
        return handle_sta_ip(remoteAddr);
    }

    const session_info_t& ESPSessionManager::new_session(const IPAddress& localIP, const uint32_t& remoteAddr, const String& id)
    {
        if (is_sta(localIP))
            return sta_sessions.emplace(remoteAddr, session_info_t(remoteAddr, *get_sta_mac(remoteAddr), WIFI_IF_STA, id)).first->second;
        auto& ap_session = ap_sessions.at(remoteAddr);
        ap_session._userID = id;
        return ap_session;
    }

    void ESPSessionManager::remove_session(const IPAddress& localIP, const uint32_t& remoteAddr)
    {
        if (is_sta(localIP))
            sta_sessions.erase(remoteAddr);
        else
            ap_sessions.at(remoteAddr)._userID = emptyString;
    }

    ESPSessionManager::ESPSessionManager()
    { // register Wi-Fi event handlers for connection and disconnection
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
            { on_sta_connect(info.wifi_ap_staipassigned.ip.addr); },
            ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
            { on_sta_disconnect(info.wifi_ap_stadisconnected.mac); },
            ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    }

    void ESPSessionManager::on_sta_connect(const uint32_t& ip)
    { // register new station IP & MAC address once device connects to AP
        wifi_sta_list_t wifi_sta_list;
        esp_netif_sta_list_t netif_sta_list;
        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list);

        /* esp_netif_sta_info_t new_sta_info;
        for (auto &&sta_info : netif_sta_list.sta)
            if (sta_info.ip.addr == ip)
            {
                new_sta_info = sta_info;
                break;
            } */
            // TODO: in C++ 20 use std::unordered_map::insert_or_assign
        auto new_sta_info = std::find_if(std::begin(netif_sta_list.sta), std::end(netif_sta_list.sta),
            [&](esp_netif_sta_info_t temp_sta_info)
            { return temp_sta_info.ip.addr == ip; });
        log_d("%u", new_sta_info->ip.addr);
        // ap_sessions.erase(new_sta_info->ip.addr);
        eth_addr temp;
        std::move(std::begin(new_sta_info->mac), std::end(new_sta_info->mac), std::begin(temp.addr)); // TODO: in C++ 20 use std::to_array
        // ap_sessions.emplace(new_sta_info->ip.addr, session_info_t(new_sta_info->ip.addr, temp, conn_mode_t::CONN_AP, emptyString));
        ap_sessions[new_sta_info->ip.addr] = { new_sta_info->ip.addr, temp, WIFI_IF_AP, emptyString };
    }

    void ESPSessionManager::on_sta_disconnect(const uint8_t(&mac)[6])
    { // remove session once a AP station disconnects from Wi-Fi
        /* for (auto client = ap_sessions.begin(); client != ap_sessions.end(); ++client)
            if (std::equal(std::begin(client->second._mac.addr), std::end(client->second._mac.addr), std::begin(mac)))
                ap_sessions.erase(client); */
                // TODO: use in C++20 std::erase_if
        auto session_to_erase = std::find_if(ap_sessions.begin(), ap_sessions.end(), [&](decltype(*ap_sessions.begin()) temp_ap_session)
            { return std::equal(std::begin(mac), std::end(mac), std::begin(temp_ap_session.second._mac.addr)); });
        if (session_to_erase != ap_sessions.end())
            ap_sessions.erase(session_to_erase);
        /* std::for_each(ap_sessions.begin(), ap_sessions.end(), [&](decltype(*ap_sessions.begin()) temp_ap_session)
                      {if (std::equal(std::begin(mac), std::end(mac), std::begin(temp_ap_session.second._mac.addr)))
                      ap_sessions.erase(temp_ap_session.first); }); */
    }

    JSONVar ESPSessionManager::to_json()
    {
        JSONVar sessions_json;
        size_t i = 0;
        for (auto it = sta_sessions.begin(); it != sta_sessions.end() && it->second; ++it, ++i)
        {
            auto temp_json = it->second.toJSON();
            sessions_json[i] = temp_json;
        }
        for (auto it = ap_sessions.begin(); it != ap_sessions.end() && it->second; ++it, ++i)
        {
            auto temp_json = it->second.toJSON();
            sessions_json[i] = temp_json;
        }
        return sessions_json;
    }

    inline bool ESPSessionManager::is_ap(const IPAddress& localIP)
    {
        return WiFi.localIP() != localIP;
    }

    inline bool ESPSessionManager::is_sta(const IPAddress& localIP)
    {
        return WiFi.localIP() == localIP;
    }

    ESPSessionManager session_manager;
} // namespace cst

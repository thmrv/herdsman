#include <stdio.h>
#include <windows.h>
#include <winerror.h>
#include <ras.h>
#include <raserror.h>
#include <ipsectypes.h>
#include <MprApi.h>

#pragma comment(lib, "rasapi32.lib")


namespace VPNCli {

#define DEFAULT_PHONE_BOOK NULL

    // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessage

    void PrintSystemError(DWORD error) {
        DWORD cBufSize = 512;
        TCHAR lpszErrorString[512];

        DWORD bufLen = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                lpszErrorString,
                cBufSize, NULL);
        if (bufLen) {
            wprintf(L"%s\n", lpszErrorString);
        }
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgeterrorstringa

    void PrintRasError(DWORD error) {
        DWORD cBufSize = 512;
        TCHAR lpszErrorString[512];

        if (error > RASBASE && error < RASBASEEND) {
            if (RasGetErrorString(error, lpszErrorString, cBufSize) == ERROR_SUCCESS) {
                wprintf(L"%s\n", lpszErrorString);
                return;
            }
        }

        PrintSystemError(error);
    }

    int PrintConnectionDetails(HRASCONN connection) {
        DWORD dwCb = 0;
        DWORD dwRet = ERROR_SUCCESS;
        PRAS_PROJECTION_INFO lpProjectionInfo = NULL;

        // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgetprojectioninfoex

        dwRet = RasGetProjectionInfoEx(connection, lpProjectionInfo, &dwCb);
        if (dwRet == ERROR_BUFFER_TOO_SMALL) {
            lpProjectionInfo = (PRAS_PROJECTION_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
            lpProjectionInfo->version = RASAPIVERSION_CURRENT;
            dwRet = RasGetProjectionInfoEx(connection, lpProjectionInfo, &dwCb);
            if (dwRet != ERROR_SUCCESS) {
                PrintRasError(dwRet);
                if (lpProjectionInfo) {
                    HeapFree(GetProcessHeap(), 0, lpProjectionInfo);
                    lpProjectionInfo = NULL;
                }
                return dwRet;
            }

            if (lpProjectionInfo->type == PROJECTION_INFO_TYPE_IKEv2) {

                wprintf(L"\ttype=PROJECTION_INFO_TYPE_IKEv2");

                // IPv4
                wprintf(L"\n\tdwIPv4NegotiationError=%d", lpProjectionInfo->ikev2.dwIPv4NegotiationError);
                wprintf(L"\n\tipv4Address=");
                wprintf(L"\n\tipv4ServerAddress=");

                // IPv6 UNUSED
                //DWORD         dwIPv6NegotiationError;
                //RASIPV6ADDR   ipv6Address;
                //RASIPV6ADDR   ipv6ServerAddress;
                //DWORD         dwPrefixLength;

                wprintf(L"\n\tdwAuthenticationProtocol=");
                if (lpProjectionInfo->ikev2.dwAuthenticationProtocol == RASIKEv2_AUTH_MACHINECERTIFICATES) wprintf(L"RASIKEv2_AUTH_MACHINECERTIFICATES");
                else if (lpProjectionInfo->ikev2.dwAuthenticationProtocol == RASIKEv2_AUTH_EAP) wprintf(L"RASIKEv2_AUTH_EAP");
                wprintf(L"\n\tdwEapTypeId=%d", lpProjectionInfo->ikev2.dwEapTypeId);

                wprintf(L"\n\tdwFlags=");
                if (lpProjectionInfo->ikev2.dwFlags & RASIKEv2_FLAGS_MOBIKESUPPORTED) wprintf(L"RASIKEv2_FLAGS_MOBIKESUPPORTED, ");
                if (lpProjectionInfo->ikev2.dwFlags & RASIKEv2_FLAGS_BEHIND_NAT) wprintf(L"RASIKEv2_FLAGS_BEHIND_NAT, ");
                if (lpProjectionInfo->ikev2.dwFlags & RASIKEv2_FLAGS_SERVERBEHIND_NAT) wprintf(L"RASIKEv2_FLAGS_SERVERBEHIND_NAT");
                wprintf(L"\n\tdwEncryptionMethod=");

                // https://docs.microsoft.com/en-us/windows/win32/api/ipsectypes/ne-ipsectypes-ipsec_cipher_type

                if (lpProjectionInfo->ikev2.dwEncryptionMethod == IPSEC_CIPHER_TYPE_DES) wprintf(L"IPSEC_CIPHER_TYPE_DES");
                else if (lpProjectionInfo->ikev2.dwEncryptionMethod == IPSEC_CIPHER_TYPE_3DES) wprintf(L"IPSEC_CIPHER_TYPE_3DES");
                else if (lpProjectionInfo->ikev2.dwEncryptionMethod == IPSEC_CIPHER_TYPE_AES_128) wprintf(L"IPSEC_CIPHER_TYPE_AES_128");
                else if (lpProjectionInfo->ikev2.dwEncryptionMethod == IPSEC_CIPHER_TYPE_AES_192) wprintf(L"IPSEC_CIPHER_TYPE_AES_192");
                else if (lpProjectionInfo->ikev2.dwEncryptionMethod == IPSEC_CIPHER_TYPE_AES_256) wprintf(L"IPSEC_CIPHER_TYPE_AES_256");
                else wprintf(L"unknown (%d)", lpProjectionInfo->ikev2.dwEncryptionMethod);

                // -
                wprintf(L"\n\tnumIPv4ServerAddresses=%d", lpProjectionInfo->ikev2.numIPv4ServerAddresses);
                wprintf(L"\n\tipv4ServerAddresses=");
                for (DWORD j = 0; j < lpProjectionInfo->ikev2.numIPv4ServerAddresses; j++) {
                    if ((j + 1) < lpProjectionInfo->ikev2.numIPv4ServerAddresses) wprintf(L", ");
                }
                wprintf(L"\n\tnumIPv6ServerAddresses=%d", lpProjectionInfo->ikev2.numIPv6ServerAddresses);
                //RASIPV6ADDR* ipv6ServerAddresses;
            } else if (lpProjectionInfo->type == PROJECTION_INFO_TYPE_PPP) {
                wprintf(L"\ttype=PROJECTION_INFO_TYPE_PPP");
            }

            HeapFree(GetProcessHeap(), 0, lpProjectionInfo);
            lpProjectionInfo = NULL;
        } else {
            wprintf(L"\tError calling RasGetProjectionInfoEx: ");
            PrintRasError(dwRet);
        }

        return dwRet;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasenumconnectionsa

    int PrintConnections() {
        DWORD dwCb = 0;
        DWORD dwRet = ERROR_SUCCESS;
        DWORD dwConnections = 0;
        LPRASCONN lpRasConn = NULL;

        dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
        if (dwRet == ERROR_BUFFER_TOO_SMALL) {
            // Allocate the memory needed for the array of RAS structure(s).
            lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
            if (lpRasConn == NULL) {
                wprintf(L"HeapAlloc failed!\n");
                return 0;
            }

            lpRasConn[0].dwSize = sizeof(RASCONN);

            dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

            if (ERROR_SUCCESS == dwRet) {
                wprintf(L"The following RAS connections are currently active:\n");
                for (DWORD i = 0; i < dwConnections; i++) {
                    wprintf(L"%s\n", lpRasConn[i].szEntryName);
                    PrintConnectionDetails(lpRasConn[i].hrasconn);
                }
            }
            wprintf(L"\n");

            HeapFree(GetProcessHeap(), 0, lpRasConn);
            lpRasConn = NULL;
            return 0;
        }


        if (dwConnections >= 1) {
            wprintf(L"The operation failed to acquire the buffer size.\n\n");
        } else {
            wprintf(L"There are no active RAS connections.\n\n");
        }

        return 0;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasenumdevicesa

    int PrintDevices() {
        DWORD dwCb = 0;
        DWORD dwRet = ERROR_SUCCESS;
        DWORD dwDevices = 0;
        LPRASDEVINFO lpRasDevInfo = NULL;


        dwRet = RasEnumDevices(lpRasDevInfo, &dwCb, &dwDevices);

        if (dwRet == ERROR_BUFFER_TOO_SMALL) {

            lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
            if (lpRasDevInfo == NULL) {
                wprintf(L"HeapAlloc failed!\n");
                return 0;
            }

            lpRasDevInfo[0].dwSize = sizeof(RASDEVINFO);

            // Call RasEnumDevices to enumerate RAS devices
            dwRet = RasEnumDevices(lpRasDevInfo, &dwCb, &dwDevices);

            if (ERROR_SUCCESS == dwRet) {
                wprintf(L"The following RAS devices were found:\n");
                for (DWORD i = 0; i < dwDevices; i++) {
                    wprintf(L"%s\n", lpRasDevInfo[i].szDeviceName);
                }
            }
            wprintf(L"\n");
            //Deallocate memory for the connection buffer
            HeapFree(GetProcessHeap(), 0, lpRasDevInfo);
            lpRasDevInfo = NULL;
            return 0;
        }

        if (dwDevices >= 1) {
            wprintf(L"The operation failed to acquire the buffer size.\n\n");
        } else {
            wprintf(L"There were no RAS devices found.\n\n");
        }

        return 0;
    }

    void PrintOptions(DWORD options) {
        wprintf(L"\tdwfOptions = {\n");
        if (options & RASEO_UseCountryAndAreaCodes) wprintf(L"\t\tRASEO_UseCountryAndAreaCodes\n");
        if (options & RASEO_SpecificIpAddr) wprintf(L"\t\tRASEO_SpecificIpAddr\n");
        if (options & RASEO_SpecificNameServers) wprintf(L"\t\tRASEO_SpecificNameServers\n");
        if (options & RASEO_IpHeaderCompression) wprintf(L"\t\tRASEO_IpHeaderCompression\n");
        if (options & RASEO_RemoteDefaultGateway) wprintf(L"\t\tRASEO_RemoteDefaultGateway\n");
        if (options & RASEO_DisableLcpExtensions) wprintf(L"\t\tRASEO_DisableLcpExtensions\n");
        if (options & RASEO_TerminalBeforeDial) wprintf(L"\t\tRASEO_TerminalBeforeDial\n");
        if (options & RASEO_TerminalAfterDial) wprintf(L"\t\tRASEO_TerminalAfterDial\n");
        if (options & RASEO_ModemLights) wprintf(L"\t\tRASEO_ModemLights\n");
        if (options & RASEO_SwCompression) wprintf(L"\t\tRASEO_SwCompression\n");
        if (options & RASEO_RequireEncryptedPw) wprintf(L"\t\tRASEO_RequireEncryptedPw\n");
        if (options & RASEO_RequireMsEncryptedPw) wprintf(L"\t\tRASEO_RequireMsEncryptedPw\n");
        if (options & RASEO_RequireDataEncryption) wprintf(L"\t\tRASEO_RequireDataEncryption\n");
        if (options & RASEO_NetworkLogon) wprintf(L"\t\tRASEO_NetworkLogon\n");
        if (options & RASEO_UseLogonCredentials) wprintf(L"\t\tRASEO_UseLogonCredentials\n");
        if (options & RASEO_PromoteAlternates) wprintf(L"\t\tRASEO_PromoteAlternates\n");

#if (WINVER >= 0x401)
        if (options & RASEO_SecureLocalFiles) wprintf(L"\t\tRASEO_SecureLocalFiles\n");
#endif

#if (WINVER >= 0x500)
        if (options & RASEO_RequireEAP) wprintf(L"\t\tRASEO_RequireEAP\n");
        if (options & RASEO_RequirePAP) wprintf(L"\t\tRASEO_RequirePAP\n");
        if (options & RASEO_RequireSPAP) wprintf(L"\t\tRASEO_RequireSPAP\n");
        if (options & RASEO_Custom) wprintf(L"\t\tRASEO_Custom\n");

        if (options & RASEO_PreviewPhoneNumber) wprintf(L"\t\tRASEO_PreviewPhoneNumber\n");
        if (options & RASEO_SharedPhoneNumbers) wprintf(L"\t\tRASEO_SharedPhoneNumbers\n");
        if (options & RASEO_PreviewUserPw) wprintf(L"\t\tRASEO_PreviewUserPw\n");
        if (options & RASEO_PreviewDomain) wprintf(L"\t\tRASEO_PreviewDomain\n");
        if (options & RASEO_ShowDialingProgress) wprintf(L"\t\tRASEO_ShowDialingProgress\n");
        if (options & RASEO_RequireCHAP) wprintf(L"\t\tRASEO_RequireCHAP\n");
        if (options & RASEO_RequireMsCHAP) wprintf(L"\t\tRASEO_RequireMsCHAP\n");
        if (options & RASEO_RequireMsCHAP2) wprintf(L"\t\tRASEO_RequireMsCHAP2\n");
        if (options & RASEO_RequireW95MSCHAP) wprintf(L"\t\tRASEO_RequireW95MSCHAP\n");
        if (options & RASEO_CustomScript) wprintf(L"\t\tRASEO_CustomScript\n");
#endif

        wprintf(L"\t};\n");
    }

    void PrintOptions2(DWORD options) {
        wprintf(L"\tdwfOptions2 = {\n");

        //WINVER check

#if (WINVER >= 0x501)
        if (options & RASEO2_SecureFileAndPrint) wprintf(L"\t\tRASEO2_SecureFileAndPrint\n");
        if (options & RASEO2_SecureClientForMSNet) wprintf(L"\t\tRASEO2_SecureClientForMSNet\n");
        if (options & RASEO2_DontNegotiateMultilink) wprintf(L"\t\tRASEO2_DontNegotiateMultilink\n");
        if (options & RASEO2_DontUseRasCredentials) wprintf(L"\t\tRASEO2_DontUseRasCredentials\n");
        if (options & RASEO2_UsePreSharedKey) wprintf(L"\t\tRASEO2_UsePreSharedKey\n");
        if (options & RASEO2_Internet) wprintf(L"\t\tRASEO2_Internet\n");
        if (options & RASEO2_DisableNbtOverIP) wprintf(L"\t\tRASEO2_DisableNbtOverIP\n");
        if (options & RASEO2_UseGlobalDeviceSettings) wprintf(L"\t\tRASEO2_UseGlobalDeviceSettings\n");
        if (options & RASEO2_ReconnectIfDropped) wprintf(L"\t\tRASEO2_ReconnectIfDropped\n");
        if (options & RASEO2_SharePhoneNumbers) wprintf(L"\t\tRASEO2_SharePhoneNumbers\n");
#endif

#if (WINVER >= 0x600)
        if (options & RASEO2_SecureRoutingCompartment) wprintf(L"\t\tRASEO2_SecureRoutingCompartment\n");
        if (options & RASEO2_UseTypicalSettings) wprintf(L"\t\tRASEO2_UseTypicalSettings\n");
        if (options & RASEO2_IPv6SpecificNameServers) wprintf(L"\t\tRASEO2_IPv6SpecificNameServers\n");
        if (options & RASEO2_IPv6RemoteDefaultGateway) wprintf(L"\t\tRASEO2_IPv6RemoteDefaultGateway\n");
        if (options & RASEO2_RegisterIpWithDNS) wprintf(L"\t\tRASEO2_RegisterIpWithDNS\n");
        if (options & RASEO2_UseDNSSuffixForRegistration) wprintf(L"\t\tRASEO2_UseDNSSuffixForRegistration\n");
        if (options & RASEO2_IPv4ExplicitMetric) wprintf(L"\t\tRASEO2_IPv4ExplicitMetric\n");
        if (options & RASEO2_IPv6ExplicitMetric) wprintf(L"\t\tRASEO2_IPv6ExplicitMetric\n");
        if (options & RASEO2_DisableIKENameEkuCheck) wprintf(L"\t\tRASEO2_DisableIKENameEkuCheck\n");
#endif

#if (WINVER >= 0x601)
        if (options & RASEO2_DisableClassBasedStaticRoute) wprintf(L"\t\tRASEO2_DisableClassBasedStaticRoute\n");
        if (options & RASEO2_SpecificIPv6Addr) wprintf(L"\t\tRASEO2_SpecificIPv6Addr\n");
        if (options & RASEO2_DisableMobility) wprintf(L"\t\tRASEO2_DisableMobility\n");
        if (options & RASEO2_RequireMachineCertificates) wprintf(L"\t\tRASEO2_RequireMachineCertificates\n");
#endif

#if (WINVER >= 0x602)
        if (options & RASEO2_UsePreSharedKeyForIkev2Initiator) wprintf(L"\t\tRASEO2_UsePreSharedKeyForIkev2Initiator\n");
        if (options & RASEO2_UsePreSharedKeyForIkev2Responder) wprintf(L"\t\tRASEO2_UsePreSharedKeyForIkev2Responder\n");
        if (options & RASEO2_CacheCredentials) wprintf(L"\t\tRASEO2_CacheCredentials\n");
#endif

#if (WINVER >= 0x603)
        if (options & RASEO2_AutoTriggerCapable) wprintf(L"\t\tRASEO2_AutoTriggerCapable\n");
        if (options & RASEO2_IsThirdPartyProfile) wprintf(L"\t\tRASEO2_IsThirdPartyProfile\n");
        if (options & RASEO2_AuthTypeIsOtp) wprintf(L"\t\tRASEO2_AuthTypeIsOtp\n");
#endif

#if (WINVER >= 0x604)
        if (options & RASEO2_IsAlwaysOn) wprintf(L"\t\tRASEO2_IsAlwaysOn\n");
        if (options & RASEO2_IsPrivateNetwork) wprintf(L"\t\tRASEO2_IsPrivateNetwork\n");
#endif

#if (WINVER >= 0xA00)
        if (options & RASEO2_PlumbIKEv2TSAsRoutes) wprintf(L"\t\tRASEO2_PlumbIKEv2TSAsRoutes\n");
#endif

        wprintf(L"\t};");
    }

    void PrintBytes(LPCWSTR name, LPBYTE bytes, DWORD len) {
        bool next_is_newline = false;
        const int bytes_per_line = 12;
        wprintf(L"\n\t[%s: %d bytes]\n\t\t", name, len);
        for (DWORD i = 0; i < len; i++) {
            if (i > 0 && !next_is_newline) {
                wprintf(L", ");
            }
            wprintf(L"0x%02x", bytes[i]);
            next_is_newline = ((i + 1) % bytes_per_line) == 0;
            if (next_is_newline) {
                wprintf(L"\n\t\t");
            }
        }
        wprintf(L"\n\t[/%s]", name);
    }

    int PrintEntryDetails(LPCTSTR entry_name) {
        DWORD dwCb = 0;
        DWORD dwRet = ERROR_SUCCESS;
        LPRASENTRY lpRasEntry = NULL;

        dwRet = RasGetEntryProperties(DEFAULT_PHONE_BOOK, entry_name, lpRasEntry, &dwCb, NULL, NULL);
        if (dwRet == ERROR_BUFFER_TOO_SMALL) {
            lpRasEntry = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
            if (lpRasEntry == NULL) {
                wprintf(L"HeapAlloc failed!\n");
                return 0;
            }

            lpRasEntry[0].dwSize = sizeof(RASENTRY);
            dwRet = RasGetEntryProperties(DEFAULT_PHONE_BOOK, entry_name, lpRasEntry, &dwCb, NULL, NULL);
            switch (dwRet) {
                case ERROR_INVALID_SIZE:
                    wprintf(L"An incorrect structure size was detected.\n");
                    break;
            }

            PrintOptions(lpRasEntry->dwfOptions);
            PrintOptions2(lpRasEntry->dwfOptions2);

            // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgetcustomauthdataa

            LPBYTE custom_auth_data = NULL;
            dwRet = RasGetCustomAuthData(DEFAULT_PHONE_BOOK, entry_name, custom_auth_data, &dwCb);
            if (dwRet == ERROR_BUFFER_TOO_SMALL && dwCb > 0) {
                custom_auth_data = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
                dwRet = RasGetCustomAuthData(DEFAULT_PHONE_BOOK, entry_name, custom_auth_data, &dwCb);
                if (dwRet != ERROR_SUCCESS) {
                    PrintRasError(dwRet);
                    if (custom_auth_data) {
                        HeapFree(GetProcessHeap(), 0, custom_auth_data);
                        custom_auth_data = NULL;
                    }
                    return dwRet;
                }
                PrintBytes(L"CustomAuthData", custom_auth_data, dwCb);
                HeapFree(GetProcessHeap(), 0, custom_auth_data);
            } else if (dwCb > 0) {
                wprintf(L"\n\tError calling RasGetCustomAuthData: ");
                PrintRasError(dwRet);
            }

            // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgeteapuserdataa

            LPBYTE eap_user_data = NULL;
            dwRet = RasGetEapUserData(NULL, DEFAULT_PHONE_BOOK, entry_name, eap_user_data, &dwCb);
            if (dwRet == ERROR_BUFFER_TOO_SMALL && dwCb > 0) {
                eap_user_data = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
                dwRet = RasGetEapUserData(NULL, DEFAULT_PHONE_BOOK, entry_name, eap_user_data, &dwCb);
                if (dwRet != ERROR_SUCCESS) {
                    PrintRasError(dwRet);
                    if (eap_user_data) {
                        HeapFree(GetProcessHeap(), 0, eap_user_data);
                        eap_user_data = NULL;
                    }
                    return dwRet;
                }
                PrintBytes(L"EapUserData", eap_user_data, dwCb);
                HeapFree(GetProcessHeap(), 0, eap_user_data);
            } else if (dwCb > 0) {
                wprintf(L"\n\tError calling RasGetEapUserData: ");
                PrintRasError(dwRet);
            }

            // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgetsubentrypropertiesa

            wprintf(L"\n\tdwSubEntries: %d", lpRasEntry->dwSubEntries);
            if (lpRasEntry->dwSubEntries > 0) {
                for (DWORD i = 0; i < lpRasEntry->dwSubEntries; i++) {
                    LPRASSUBENTRY lpRasSubEntry = NULL;
                    dwRet = RasGetSubEntryProperties(DEFAULT_PHONE_BOOK, entry_name, i + 1, lpRasSubEntry, &dwCb, NULL, NULL);
                    if (dwRet == ERROR_BUFFER_TOO_SMALL && dwCb > 0) {
                        lpRasSubEntry = (LPRASSUBENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
                        dwRet = RasGetSubEntryProperties(DEFAULT_PHONE_BOOK, entry_name, i + 1, lpRasSubEntry, &dwCb, NULL, NULL);
                        if (dwRet != ERROR_SUCCESS) {
                            PrintRasError(dwRet);
                            if (lpRasSubEntry) {
                                HeapFree(GetProcessHeap(), 0, lpRasSubEntry);
                                lpRasSubEntry = NULL;
                            }
                            return dwRet;
                        }
                        wprintf(L"\n\t\tdwSize=%d", lpRasSubEntry->dwSize);
                        wprintf(L"\n\t\tdwfFlags=%d", lpRasSubEntry->dwfFlags);
                        wprintf(L"\n\t\tszDeviceType=%s", lpRasSubEntry->szDeviceType);
                        wprintf(L"\n\t\tszDeviceName=%s", lpRasSubEntry->szDeviceName);
                        wprintf(L"\n\t\tszLocalPhoneNumber=%s", lpRasSubEntry->szLocalPhoneNumber);
                        wprintf(L"\n\t\tdwAlternateOffset=%d", lpRasSubEntry->dwAlternateOffset);
                        HeapFree(GetProcessHeap(), 0, lpRasSubEntry);
                        lpRasSubEntry = NULL;
                    } else {
                        wprintf(L"\n\tError calling RasGetSubEntryProperties: ");
                        PrintRasError(dwRet);
                    }
                }
            }

            wprintf(L"\n");
            //Deallocate memory for the entry buffer
            HeapFree(GetProcessHeap(), 0, lpRasEntry);
            lpRasEntry = NULL;
            return ERROR_SUCCESS;
        }

        return dwRet;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasenumentriesa

    int PrintEntries() {
        DWORD dwCb = 0;
        DWORD dwRet = ERROR_SUCCESS;
        DWORD dwEntries = 0;
        LPRASENTRYNAME lpRasEntryName = NULL;


        dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);
        if (dwRet == ERROR_BUFFER_TOO_SMALL) {
            // Allocate
            lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
            if (lpRasEntryName == NULL) {
                wprintf(L"HeapAlloc failed!\n");
                return 0;
            }
            lpRasEntryName[0].dwSize = sizeof(RASENTRYNAME);

            // Call RasEnumEntries to enumerate all RAS entry names
            dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);

            // If successful, print the RAS entry names
            if (ERROR_SUCCESS == dwRet) {
                wprintf(L"The following RAS entry names were found:\n");
                for (DWORD i = 0; i < dwEntries; i++) {
                    wprintf(L"%s\n", lpRasEntryName[i].szEntryName);
                    dwRet = PrintEntryDetails(lpRasEntryName[i].szEntryName);
                }
            }
            //Deallocate memory for the connection buffer
            HeapFree(GetProcessHeap(), 0, lpRasEntryName);
            lpRasEntryName = NULL;
            return ERROR_SUCCESS;
        }

        // There was either a problem with RAS or there are RAS entry names to enumerate
        if (dwEntries >= 1) {
            wprintf(L"The operation failed to acquire the buffer size.\n\n");
        } else {
            wprintf(L"There were no RAS entry names found:.\n\n");
        }

        return dwRet;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rassetcredentialsa

    DWORD SetCredentials(LPCTSTR entry_name, LPCTSTR username, LPCTSTR password) {
        RASCREDENTIALS credentials;

        ZeroMemory(&credentials, sizeof(RASCREDENTIALS));
        credentials.dwSize = sizeof(RASCREDENTIALS);
        credentials.dwMask = RASCM_UserName | RASCM_Password;

        wcscpy_s(reinterpret_cast<wchar_t *>(credentials.szUserName), 256, reinterpret_cast<const wchar_t *>(username));
        wcscpy_s(reinterpret_cast<wchar_t *>(credentials.szPassword), 256, reinterpret_cast<const wchar_t *>(password));

        DWORD dwRet = RasSetCredentials(DEFAULT_PHONE_BOOK, entry_name, &credentials, FALSE);
        if (dwRet != ERROR_SUCCESS) {
            PrintRasError(dwRet);
            return dwRet;
        }

        return ERROR_SUCCESS;
    }

// https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rassetentrypropertiesa
    DWORD CreateEntry(LPCTSTR entry_name, LPCTSTR hostname, LPCTSTR username, LPCTSTR password) {
        RASENTRY entry;
        ZeroMemory(&entry, sizeof(RASENTRY));

        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa377274(v=vs.85)

        entry.dwSize = sizeof(RASENTRY);
        entry.dwfOptions = RASEO2_RequireMachineCertificates | RASEO_RemoteDefaultGateway | RASEO_PreviewUserPw | RASEO_PreviewDomain;
        wcscpy_s(reinterpret_cast<wchar_t *>(entry.szLocalPhoneNumber), 128,
                 reinterpret_cast<const wchar_t *>(hostname));
        entry.dwfNetProtocols = RASNP_Ip | RASNP_Ipv6;
        entry.dwFramingProtocol = RASFP_Ppp;
        wcscpy_s(reinterpret_cast<wchar_t *>(entry.szDeviceType), 16, reinterpret_cast<const wchar_t *>(RASDT_Vpn));
        wcscpy_s(reinterpret_cast<wchar_t *>(entry.szDeviceName), 128,
                 reinterpret_cast<const wchar_t *>("WAN Miniport (IKEv2)"));
        entry.dwType = RASET_Vpn;
        entry.dwEncryptionType = ET_Optional;
        entry.dwVpnStrategy = VS_Ikev2Only;
        entry.dwfOptions2 = RASEO2_DontNegotiateMultilink | RASEO2_ReconnectIfDropped | RASEO2_IPv6RemoteDefaultGateway | RASEO2_CacheCredentials;
        entry.dwRedialCount = 9;
        entry.dwRedialPause = 30;

        // this maps to "Type of sign-in info" => "User name and password"
        entry.dwCustomAuthKey = 26;

        DWORD dwRet = RasSetEntryProperties(DEFAULT_PHONE_BOOK, entry_name, &entry, entry.dwSize, NULL, NULL);
        if (dwRet != ERROR_SUCCESS) {
            PrintRasError(dwRet);
            return dwRet;
        }

        dwRet = SetCredentials(entry_name, username, password);

        // https://docs.microsoft.com/en-us/windows/win32/api/processenv/nf-processenv-expandenvironmentstringsa
        wchar_t AppDataPath[1025] = { 0 };
        dwRet = ExpandEnvironmentStrings(TEXT("%APPDATA%"), reinterpret_cast<LPSTR>(AppDataPath), 1024);
        if (dwRet == 0) {
            PrintRasError(GetLastError());
        }

        wchar_t PhonebookPath[2048] = { 0 };
        swprintf(PhonebookPath, 2048, L"%s\\Microsoft\\Network\\Connections\\Pbk\\rasphone.pbk", AppDataPath);

        // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-writeprivateprofilestringw
        BOOL wrote_entry = WritePrivateProfileString(
                entry_name,
                reinterpret_cast<LPCSTR>(L"NumCustomPolicy"),
                reinterpret_cast<LPCSTR>(L"1"),
                NULL
        );
        if (!wrote_entry) {
            wprintf(L"ERROR: failed to write \"NumCustomPolicy\" field to `%s`", PhonebookPath);
        }

        wrote_entry = WritePrivateProfileString(
                entry_name,
                reinterpret_cast<LPCSTR>(L"CustomIPSecPolicies"),
                reinterpret_cast<LPCSTR>(L"030000000400000002000000050000000200000000000000"),
                NULL
        );
        if (!wrote_entry) {
            wprintf(L"ERROR: failed to write \"CustomIPSecPolicies\" field to `%s`", PhonebookPath);
        }

        return ERROR_SUCCESS;
    }

    DWORD RemoveEntry(LPCTSTR entry_name) {
        DWORD dwRet = RasDeleteEntry(DEFAULT_PHONE_BOOK, entry_name);
        if (dwRet != ERROR_SUCCESS) {
            PrintRasError(dwRet);
            return dwRet;
        }
        return dwRet;
    }

// https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasdiala
    DWORD Connect(LPCTSTR entry_name) {
        LPRASDIALPARAMS lpRasDialParams = NULL;
        DWORD cb = sizeof(RASDIALPARAMS);

        lpRasDialParams = (LPRASDIALPARAMS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if (lpRasDialParams == NULL) {
            wprintf(L"HeapAlloc failed!\n");
            return 0;
        }
        lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
        wcscpy_s(reinterpret_cast<wchar_t *>(lpRasDialParams->szEntryName), 256,
                 reinterpret_cast<const wchar_t *>(entry_name));
        wcscpy_s(reinterpret_cast<wchar_t *>(lpRasDialParams->szDomain), 15, L"*");
        // https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rasgetcredentialsw
        RASCREDENTIALS credentials;

        ZeroMemory(&credentials, sizeof(RASCREDENTIALS));
        credentials.dwSize = sizeof(RASCREDENTIALS);
        credentials.dwMask = RASCM_UserName | RASCM_Password;
        DWORD dwRet = RasGetCredentials(DEFAULT_PHONE_BOOK, entry_name, &credentials);
        if (dwRet != ERROR_SUCCESS) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);
            PrintRasError(dwRet);
            return dwRet;
        }
        wcscpy_s(reinterpret_cast<wchar_t *>(lpRasDialParams->szUserName), 256,
                 reinterpret_cast<const wchar_t *>(credentials.szUserName));
        wcscpy_s(reinterpret_cast<wchar_t *>(lpRasDialParams->szPassword), 256,
                 reinterpret_cast<const wchar_t *>(credentials.szPassword));

        wprintf(L"Connecting to `%s`...\n", entry_name);
        HRASCONN hRasConn = NULL;
        dwRet = RasDial(NULL, DEFAULT_PHONE_BOOK, lpRasDialParams, NULL, NULL, &hRasConn);
        if (dwRet != ERROR_SUCCESS) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);
            PrintRasError(dwRet);
            return dwRet;
        }
        wprintf(L"SUCCESS!\n");

        // store handle if needed, etc
        //..

        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);

        return ERROR_SUCCESS;
    }

// https://docs.microsoft.com/en-us/windows/win32/api/ras/nf-ras-rashangupa
    DWORD Disconnect(LPCTSTR entry_name) {
        return ERROR_SUCCESS;
    }

    int AddVPNConn(wchar_t* hostname, wchar_t* entry_name, wchar_t* username, wchar_t* password) {
        //entry_name[257] = { 0 }, hostname[129] = { 0 }, username[257] = { 0 }, password[257] = { 0 };
        LPCTSTR entry_namew = reinterpret_cast<LPCTSTR>(entry_name);
        LPCTSTR hostnamew = reinterpret_cast<LPCTSTR>(hostname);
        LPCTSTR usernamew = reinterpret_cast<LPCTSTR>(username);
        LPCTSTR passwordw = reinterpret_cast<LPCTSTR>(password);

        CreateEntry(reinterpret_cast<LPCTSTR>(entry_namew), reinterpret_cast<LPCTSTR>(hostnamew),
                                      reinterpret_cast<LPCTSTR>(usernamew), reinterpret_cast<LPCTSTR>(passwordw));
        return 0;
    }

    int ExecVPNConn(wchar_t * entry_name) {
        entry_name[257] = { 0 };
        LPCTSTR entry_namew = reinterpret_cast<LPCTSTR>(entry_name);
        Connect(entry_namew);
        return 0;
    }

    int CloseVPNConn(wchar_t * entry_name) {
        entry_name[257] = { 0 };
        LPCTSTR entry_namew = reinterpret_cast<LPCTSTR>(entry_name);

        Disconnect(entry_namew);
        return 0;
    }

    int RemoveVPNConn(wchar_t * entry_name) {
        entry_name[257] = { 0 };
        LPCTSTR entry_namew = reinterpret_cast<LPCTSTR>(entry_name);

        RemoveEntry(entry_namew);
        return 0;
    }


}

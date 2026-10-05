#pragma once

#include "nvhttp.h"
#include "nvaddress.h"

#include <QThread>
#include <QReadWriteLock>
#include <QSettings>
#include <QRunnable>

class CopySafeReadWriteLock : public QReadWriteLock
{
public:
    CopySafeReadWriteLock() = default;

    // Don't actually copy the QReadWriteLock
    CopySafeReadWriteLock(const CopySafeReadWriteLock&) : QReadWriteLock() {}
    CopySafeReadWriteLock& operator=(const CopySafeReadWriteLock &) { return *this; }
};

class NvComputer
{
    friend class PcMonitorThread;
    friend class ComputerManager;
    friend class PendingQuitTask;

private:
    void sortAppList();

    bool updateAppList(QVector<NvApp> newAppList);

    bool pendingQuit;

public:
    NvComputer() = default;

    // Caller is responsible for synchronizing read access to the other host
    NvComputer(const NvComputer&) = default;

    // Caller is responsible for synchronizing read access to the other host
    NvComputer& operator=(const NvComputer &) = default;

    explicit NvComputer(NvHTTP& http, QString serverInfo);

    explicit NvComputer(QSettings& settings);

    void
    setRemoteAddress(QHostAddress);

    bool
    update(const NvComputer& that);

    bool
    wake() const;

    enum ReachabilityType
    {
        RI_UNKNOWN,
        RI_LAN,
        RI_VPN,
    };

    ReachabilityType
    getActiveAddressReachability() const;

    QVector<NvAddress>
    uniqueAddresses() const;

    void
    serialize(QSettings& settings, bool serializeApps) const;

    // Caller is responsible for synchronizing read access to both hosts
    bool
    isEqualSerialized(const NvComputer& that) const;

    enum PairState
    {
        PS_UNKNOWN,
        PS_PAIRED,
        PS_NOT_PAIRED
    };

    enum ComputerState
    {
        CS_UNKNOWN,
        CS_ONLINE,
        CS_OFFLINE
    };

    // Ephemeral traits
    ComputerState state;
    PairState pairState;
    NvAddress activeAddress;
    uint16_t activeHttpsPort;
    int currentGameId;
    QString currentGameUuid;
    QString gfeVersion;
    QString appVersion;
    QVector<NvDisplayMode> displayModes;
    int maxLumaPixelsHEVC;
    int serverCodecModeSupport;
    QString gpuModel;
    bool isSupportedServerVersion;

    // Apollo extensions
    enum Permission : int
    {
        PERM_INPUT_CONTROLLER = 0x00000100,
        PERM_INPUT_TOUCH      = 0x00000200,
        PERM_INPUT_PEN        = 0x00000400,
        PERM_INPUT_MOUSE      = 0x00000800,
        PERM_INPUT_KBD        = 0x00001000,
        PERM_CLIPBOARD_SET    = 0x00010000,
        PERM_CLIPBOARD_READ   = 0x00020000,
        PERM_FILE_UPLOAD      = 0x00040000,
        PERM_FILE_DOWNLOAD    = 0x00080000,
        PERM_SERVER_CMD       = 0x00100000,
        PERM_LIST             = 0x01000000,
        PERM_VIEW             = 0x02000000,
        PERM_LAUNCH           = 0x04000000,
    };

    // -1 if the host doesn't report permissions (non-Apollo hosts)
    int permission;
    bool vDisplaySupported;
    bool vDisplayDriverReady;
    QStringList serverCommands;

    // Apollo is the only host that reports client permissions
    bool isApolloHost() const
    {
        return permission >= 0;
    }

    bool hasPermission(int perm) const
    {
        // Hosts that don't report permissions don't restrict anything
        return permission < 0 || (permission & perm) != 0;
    }

    // Persisted traits
    NvAddress localAddress;
    NvAddress remoteAddress;
    NvAddress ipv6Address;
    NvAddress manualAddress;
    QByteArray macAddress;
    QString name;
    bool hasCustomName;
    QString uuid;
    QSslCertificate serverCert;
    QVector<NvApp> appList;
    bool isNvidiaServerSoftware;
    // Remember to update isEqualSerialized() when adding fields here!

    // Synchronization
    mutable CopySafeReadWriteLock lock;

private:
    uint16_t externalPort;
};

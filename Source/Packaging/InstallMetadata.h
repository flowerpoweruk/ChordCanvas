#pragma once
#include "Transaction.h"
#include "RegistryImage.h"

namespace cc::packaging {
enum class MetadataBoundary { descriptorStaged,previousRetained,directoryCreated,helperCopied,registryRestored,retirementMarked,registrySnapshotRemoved,descriptorRemoved,auxiliaryRemoved };
// Production callers resolve the global known folder and fixed HKLM uninstall
// key. Test callers provide isolated folders and generated HKCU keys. Mutable
// logs/progressions never enter this immutable directory participant.
class InstallMetadata final:public TransactionParticipant {
public:
    InstallMetadata(std::filesystem::path directory,HKEY hive,std::wstring registryKey,
        std::filesystem::path helper,std::string version,std::function<void(MetadataBoundary)> injection={});
    std::string stage(const std::string& token) override;
    void verify(const std::string& token,const std::string& digest) override;
    void apply(const std::string& token,const std::string& digest,const std::string& version) override;
    void validate(const std::string& token,const std::string& digest,const std::string& version) override;
    void rollback(const std::string& token,const std::string& digest) override;
    void cleanup(const std::string& token,const std::string& digest,bool committed) override;
    void retire(const std::string& token,const std::string& digest) override;
    void checkExisting(const std::string& version) override;
    void checkUninstall(const std::string& version);
    void seal(); // Called only after Inno has finalised its uninstaller/registration.
    static const std::vector<std::wstring>& registrationNames();
private:
    std::filesystem::path directory,auxiliary,retired,helper;
    HKEY hive;
    std::wstring key;
    std::string version;
    std::function<void(MetadataBoundary)> injection; // Disposable tests only; native packages supply none.
    RegistryImage descriptor(const std::string& token,const std::string& digest,bool retiring=false) const;
    void checkRegistration(const std::string& expected) const;
};
}

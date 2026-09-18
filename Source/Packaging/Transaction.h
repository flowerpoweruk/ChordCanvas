#pragma once
#include "Payload.h"
#include <functional>
#include <memory>

namespace cc::packaging {
enum class InstallMode { setup,update };
enum class InstallResult { installed,alreadyCurrent };
// Test-only injection points execute after real operations. Production supplies none.
enum class Boundary { staged,previousRetained,replaced,validated,participantApplied };
// A complete installer supplies its durable metadata/uninstaller participant.
// stage writes only recovery data and returns its SHA256; apply may then mutate
// installation metadata. Recovery always receives the pinned digest. Cleanup
// must remain bounded and preserve unknown files. All methods run on the single
// installer thread holding the mutex, never on a plug-in audio thread.
class TransactionParticipant {
public:
    virtual ~TransactionParticipant()=default;
    virtual std::string stage(const std::string& token)=0;
    // Read-only verification runs before recovery can change either participant.
    virtual void verify(const std::string& token,const std::string& digest)=0;
    virtual void apply(const std::string& token,const std::string& digest,const std::string& version)=0;
    virtual void validate(const std::string& token,const std::string& digest,const std::string& version)=0;
    virtual void rollback(const std::string& token,const std::string& digest)=0;
    // cleanup retains the descriptor until the primary marker has been removed;
    // retire may then discard it. A process interrupted between the two leaves
    // at most one recognised orphan descriptor for stage to clean up safely.
    virtual void cleanup(const std::string& token,const std::string& digest,bool committed)=0;
    virtual void retire(const std::string& token,const std::string& digest)=0;
    virtual void checkExisting(const std::string& version)=0;
};
// Holds the mutex and previous bundle through installer metadata finalisation.
// Explicit rollback reports failures; destructor rollback is best effort.
class BundleTransaction {
public:
    static std::unique_ptr<BundleTransaction> begin(const std::filesystem::path& source,
        const std::filesystem::path& parent,InstallMode mode,const std::function<void(Boundary)>& injection={},
        std::shared_ptr<TransactionParticipant> participant={});
    ~BundleTransaction();
    BundleTransaction(const BundleTransaction&)=delete;
    BundleTransaction& operator=(const BundleTransaction&)=delete;
    InstallResult result() const noexcept;
    void commit();
    void rollback();
private:
    struct State;
    std::unique_ptr<State> state;
    explicit BundleTransaction(std::unique_ptr<State>);
};
// Unknown files/reparse points stop recovery. Complete installers must also
// recover their metadata participants from matching durable snapshots.
void recoverInterruptedBundle(const std::filesystem::path& parent,std::shared_ptr<TransactionParticipant> participant={});
InstallResult installBundle(const std::filesystem::path& source,const std::filesystem::path& parent,
    InstallMode mode,const std::function<void(Boundary)>& injection={});
}

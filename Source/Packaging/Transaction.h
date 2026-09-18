#pragma once
#include "Payload.h"
#include <functional>
#include <memory>

namespace cc::packaging {
enum class InstallMode { setup,update };
enum class InstallResult { installed,alreadyCurrent };
// Test-only injection points execute after real operations. Production supplies none.
enum class Boundary { staged,previousRetained,replaced,validated };
// Holds the mutex and previous bundle through installer metadata finalisation.
// Explicit rollback reports failures; destructor rollback is best effort.
class BundleTransaction {
public:
    static std::unique_ptr<BundleTransaction> begin(const std::filesystem::path& source,
        const std::filesystem::path& parent,InstallMode mode,const std::function<void(Boundary)>& injection={});
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
void recoverInterruptedBundle(const std::filesystem::path& parent);
InstallResult installBundle(const std::filesystem::path& source,const std::filesystem::path& parent,
    InstallMode mode,const std::function<void(Boundary)>& injection={});
}

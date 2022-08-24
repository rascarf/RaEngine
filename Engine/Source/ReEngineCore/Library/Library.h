#pragma once
#include "Core/Core.h"
#include "Core/SIngletonTemplate.h"
#include <unordered_map>

namespace ReEngine
{
    template <template<typename> typename Derived,typename LibType>
    class LibraryBase : public SingletonTemplate<Derived<LibType>>
    {
    public:
        void Add(const std::string& name,const Ref<LibType>& mem)
        {
            RE_CORE_ASSERT(mLibrary.find(name) == mLibrary.end() && "Already have this member in Library!");
            mLibrary[name] = mem;
        }
        void Set(const std::string& name,const Ref<LibType>& mem)
        {
            RE_CORE_ASSERT(mLibrary.find(name) != mLibrary.end() && "Not have this member in Library!");
            mLibrary[name] = mem;
        }
        [[nodiscard]] Ref<LibType> Get(const std::string& name)
        {
            HE_CORE_ASSERT(mLibrary.find(name) != mLibrary.end() && "Can't find this member in Library!");
            return mLibrary[name];
        }

    protected:
        std::unordered_map<std::string,Ref<LibType>> mLibrary;
    };

    template<typename LibType>
    class Library:public LibrarayBase<Library,LibType>
    {
    public:
        Library();
    };
}

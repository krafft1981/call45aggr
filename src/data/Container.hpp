#include <map>
#include <vector>
#include "BaseData.hpp"
#include "systems/Singleton.h"

class CallLegLinkOnlineList;

struct Container : public Singleton<Container> {

public:
    template<typename ListType>
    using Data = DataAccessor<ListType>;

    Container();

    // Загружены хотя бы все начальные данные
    bool loaded();
    void loadall();

    std::vector< std::pair<char const*, std::shared_ptr<BaseData>> > all_data();
    
private:
    bool loaded_;
};

#include <csv_storage.h>
#include <event_dispatcher.h>


class SelectionLogger {
    public:
        SelectionLogger();
    
    private:
        void handleEvent(const std::map<std::string, std::string>& data);
};

#include <csignal>
#include <dots/Application.h>
#include <dots/tools/logging.h>
#include <Basement.h>
#include <LivingRoom.h>
#include <Stairwell.h>

int main(int argc, char* argv[])
{
    using namespace dots::type::literals;
    const std::string AppName = "smart-home";

    try
    {
        // create application
        static dots::Application App(AppName, argc, argv);
        std::signal(SIGINT, [](int) { App.exit(); });

        LOG_INFO_S("started " << AppName);

        examples::Basement basement{ 30s };
        examples::LivingRoom livingRoom;
        examples::Stairwell stairwell;

        // execute event loop of application 
        return App.exec();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR_S("ERROR running " << AppName << " -> " << e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_ERROR_S("ERROR running " << AppName << " -> <unknown exception>");
        return EXIT_FAILURE;
    }
}

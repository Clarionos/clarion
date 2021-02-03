#include <fc/log/appender.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/logger_config.hpp>


namespace fc {

   static bool reg_console_appender = log_config::register_appender<console_appender>( "console" );

} // namespace fc

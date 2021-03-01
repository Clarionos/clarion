#pragma once
#include <appbase/application.hpp>
#include <boost/asio.hpp>

namespace cl { namespace net {

namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;
using std::string;
namespace asio = boost::asio;

class net_plugin : public appbase::plugin<net_plugin>
{
   public:
     net_plugin(){};
     ~net_plugin(){};

     APPBASE_PLUGIN_REQUIRES( /*(chain_plugin)*/ );

     virtual void set_program_options( options_description& cli, options_description& cfg ) override;
     void plugin_initialize( const variables_map& options );
     void plugin_startup();
     void plugin_shutdown();

     private:
        asio::ip::address      _ip;
        uint16_t               _port = 8090;
        string                 _docroot;
        std::vector<string>    _remote_endpoints; /// ip:port to maintain connections with

};

} } // cl::net

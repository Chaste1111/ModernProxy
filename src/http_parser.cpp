#include"http_parser.h"


bool HttpParser::parse(std::string_view data,HttpRequest& req)
{
   auto p=data.find("\r\n");
   if(p==data.npos)
   {
    return false;
   }


   auto line=data.substr(0,p);
   auto a=line.find(' ');
   auto b=line.find(' ',a+1);

   if(a==line.npos||b==line.npos)
   {
    return false;
   }

   req.method=line.substr(0,a);
   req.url=line.substr(a+1,b-a-1);

   auto host_pos=data.find("Host: ");
    if(host_pos==data.npos)
    {
        return false;
    }

    auto host_end=data.find("\r\n",host_pos);
    if(host_end==data.npos)
    {
        return false;
    }

    auto host_val=host_pos+6;
    req.host=data.substr(host_val,host_end-host_val);


   return true;

}
#include "my_test.hpp"
#include "portapack.hpp"
#include <cstring>

using namespace portapack;

namespace ui
{

    NewAppView::NewAppView(NavigationView &nav) // Application Main
    {
         // App code
        
        add_children({                                       // Add pointers for widgets
            &button_helloWorld
        });

        button_helloWorld.on_select = [this](Button &){      // Button logic
            // 点击按钮
            
            std::string debug_string = "点击hello word按钮\n";
            UsbSerialAsyncmsg::asyncmsg(debug_string);
            std::vector<ui::ColorRGB888> row(ui::screen_width);
            debug_string = "初始阶段变量row的数据为："+std::to_string(row.size());
            UsbSerialAsyncmsg::asyncmsg(debug_string);
            // 目的是获取屏幕头10行的像素点
            debug_string = "###########################################\n";
            UsbSerialAsyncmsg::asyncmsg(debug_string);

           

            // 读取前10行的数据
            for(int y=0;y<1;y++)
            {
                char each_line[320*3*2+1]="\0";
                // 此函数有错误，会重复打印
                portapack::display.read_pixels({0, y, ui::screen_width, 1}, row);
                
                for(int i=0;i<10;i++)
                {
                    int tmp_offset = i*6;
                    std::string tmp_str ="idx "+ std::to_string(i) + " "+"r : "+ std::to_string(row[i].r)+" g : "+ std::to_string(row[i].g)+" b : "+ std::to_string(row[i].b)+
                    snprintf(&each_line[tmp_offset],7,"%02x%02x%02x",row[i].r,row[i].g,row[i].b);
                }
                
                debug_string = each_line;
                UsbSerialAsyncmsg::asyncmsg(debug_string);
                debug_string = "\n";
                UsbSerialAsyncmsg::asyncmsg(debug_string);
            }
            

            debug_string = "###########################################\n";
            UsbSerialAsyncmsg::asyncmsg(debug_string);
            
        
        };                                                  

    }

    void NewAppView::update()                   // Every time you get a DisplayFrameSync message this function will be ran
    {
         // Message code
    }
}
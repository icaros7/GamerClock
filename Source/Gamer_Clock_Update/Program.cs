using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Diagnostics;
using System.Windows.Forms;

namespace Gamer_Clock_Update
{
    class Program
    {
        static void Main(string[] args)
        {
            DialogResult dialogResult = MessageBox.Show("Gamer Clock 을 업데이트 하시겠습니까?", "Gamer Clock Update", MessageBoxButtons.YesNo);
            if (dialogResult == DialogResult.Yes)
            {
                if (System.IO.File.Exists("Gamer_Clock.exe"))
                {
                    foreach (var process in Process.GetProcessesByName("Gamer_Clock"))
                    {
                        process.Kill();
                    }
                    System.Threading.Thread.Sleep(1000);
                    System.IO.File.Delete("Gamer_Clock.exe");
                    WebClient we = new WebClient();
                    we.DownloadFile("https://raw.githubusercontent.com/icaros7/GamerClock/master/Gamer%20Clock/Gamer_Clock.exe", @"Gamer_Clock.exe");
                    string gc = System.AppDomain.CurrentDomain.BaseDirectory + "Gamer_Clock.exe";
                    Process.Start(gc);
                }
                else
                {
                    WebClient we = new WebClient();
                    we.DownloadFile("https://raw.githubusercontent.com/icaros7/GamerClock/master/Gamer%20Clock/Gamer_Clock.exe", @"Gamer_Clock.exe");
                    string gc = System.AppDomain.CurrentDomain.BaseDirectory + "Gamer_Clock.exe";
                    Process.Start(gc);
                }
            }
            else if (dialogResult == DialogResult.No)
            {
                string gc = System.AppDomain.CurrentDomain.BaseDirectory + "Gamer_Clock.exe";
                Process.Start(gc);
            }
            
        }
    }
}

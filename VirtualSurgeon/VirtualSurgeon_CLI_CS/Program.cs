using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VirtualSurgeon_CLI_CS
{
    class Program
    {
        static void Main(string[] args)
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonWrapper wrapper = new VirtualSurgeon_CSWrapper.VirtualSurgeonWrapper();
            wrapper.Initialize(args[0]);

            System.Console.WriteLine("initialized");
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiMessageReceivedEventArgs
    {
        public byte Message;
        public byte Data1;
        public byte Data2;

        public MidiMessageReceivedEventArgs(byte message, byte data1, byte data2)
        {
            Message = message;
            Data1 = data1;
            Data2 = data2;
        }
    }
}

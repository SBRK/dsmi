using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiMessage : IMidiMessage
    {
        public byte Message { get; set; }
        public byte Data1 { get; set; }
        public byte Data2 { get; set; }

        public MidiMessage(byte message, byte data1, byte data2)
        {
            Message = message;
            Data1 = data1;
            Data2 = data2;
        }
    }
}

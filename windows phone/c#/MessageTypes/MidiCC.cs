using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiCC
    {
        public byte Channel { get; set; }
        public byte ControlNumber { get; set; }
        public byte Value { get; set; }

        public MidiCC(byte channel, byte controlNumber, byte value)
        {
            Channel = channel;
            ControlNumber = controlNumber;
            Value = value;
        }
    }
}

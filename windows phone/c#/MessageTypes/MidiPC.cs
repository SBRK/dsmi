using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiPC
    {
        public byte Channel { get; set; }
        public byte ProgramNumber { get; set; }

        public MidiPC(byte channel, byte programNumber)
        {
            Channel = channel;
            ProgramNumber = programNumber;
        }
    }
}

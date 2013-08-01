using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiProgramChange : MidiMessage
    {
        public byte Channel
        {
            get
            {
                return (byte)(Message & 0x0F);
            }
            set
            {
                Message = (byte)(MidiMessageType.MIDI_PC | (value & 0x0F));
            }
        }

        public byte ProgramNumber
        {
            get
            {
                return Data1;
            }
            set
            {
                Data1 = value;
            }
        }

        public MidiProgramChange(byte channel, byte programNumber)
            : base((byte)(MidiMessageType.MIDI_CC | channel), programNumber, 0)
        {
        }
    }
}

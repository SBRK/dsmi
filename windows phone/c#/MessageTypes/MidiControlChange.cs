using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiControlChange : MidiMessage
    {
        public byte Channel
        {
            get
            {
                return (byte)(Message & 0x0F);
            }
            set
            {
                Message = (byte)(MidiMessageType.MIDI_CC | (value & 0x0F));
            }
        }

        public byte ControlNumber
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

        public byte Value
        {
            get
            {
                return Data2;
            }
            set
            {
                Data2 = value;
            }
        }

        public MidiControlChange(byte channel, byte controlNumber, byte value)
            : base((byte)(MidiMessageType.MIDI_CC | (channel & 0x0F)), controlNumber, value)
        {}
    }
}

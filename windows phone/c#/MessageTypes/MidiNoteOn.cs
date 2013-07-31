using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiNoteOn
    {
        public byte Channel { get; set; }
        public byte NoteNumber { get; set; }
        public byte Velocity { get; set; }

        public MidiNoteOn(byte channel, byte noteNumber, byte velocity)
        {
            Channel = channel;
            NoteNumber = noteNumber;
            Velocity = velocity;
        }
    }
}

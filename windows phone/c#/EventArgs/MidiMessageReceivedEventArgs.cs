using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiMessageReceivedEventArgs : EventArgs
    {
        public MidiMessage MidiMessage;

        public MidiMessageReceivedEventArgs(MidiMessage message)
        {
            MidiMessage = message;
        }
    }
}

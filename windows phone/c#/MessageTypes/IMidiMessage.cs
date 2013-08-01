using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public interface IMidiMessage
    {
        byte Message { get; set;}
        byte Data1 { get; set;}
        byte Data2 { get; set;}
    }
}

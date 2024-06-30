using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chronomarker.ViewModels;

internal class LogModel : ViewModelBase
{
    public ObservableCollection<string> Lines { get; set; } = [
    ];
}

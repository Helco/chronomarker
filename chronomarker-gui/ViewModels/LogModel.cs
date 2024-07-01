using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ReactiveUI;

namespace Chronomarker.ViewModels;

internal class LogModel : ViewModelBase
{
    public ObservableCollection<string> Lines { get; } = [
    ];

    public LogModel()
    {
        Lines.CollectionChanged += (_0, _1) =>
        {
            this.RaisePropertyChanged(nameof(Lines));
        };
    }
}

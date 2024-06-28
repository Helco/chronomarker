namespace Chronomarker;

// taken from: https://stackoverflow.com/a/78502696
// modified to support two values

public class MyProgressBar : ProgressBar
{
    private SolidBrush _brush = new(Color.Green);
    private SolidBrush _brush2 = new(Color.Orange);
    private SolidBrush _bkgr = new(SystemColors.Control);
    private Pen _border = new(SystemColors.ControlDarkDark, 2);
    public MyProgressBar()
    {
        // Setting these styles does the trick of sending paint message to OnPaint().
        // I did not spend any time understanding the details of why :-)
        SetStyle(ControlStyles.AllPaintingInWmPaint |
                 ControlStyles.UserPaint |
                 ControlStyles.OptimizedDoubleBuffer |
                 ControlStyles.ResizeRedraw,
                 true);
    }
    protected override void OnPaint(PaintEventArgs e)
    {
        // base.OnPaint(e);    // Don't call base class.

        // Make sure to use a float. In my case I needed to show a
        // percentage, which didn't work using the second downvoted answer.
        float fillRatio = (float)Value / (float)Maximum;
        float fillRatio2 = (float)Value2 / (float)Maximum;
        int fillTo = (int)((float)Width * fillRatio);
        int fillTo2 = (int)((float)Width * fillRatio2);

        e.Graphics.FillRectangle(_bkgr, 0, 0, Width, Height);
        e.Graphics.FillRectangle(_brush, 0, 0, fillTo, Height);
        e.Graphics.FillRectangle(_brush2, Width - fillTo2, 0, fillTo2, Height);
        e.Graphics.DrawRectangle(_border, 0, 0, Width, Height);
    }
    public new int Value
    {
        get { return base.Value; }
        set { base.Value = value; Invalidate(); }    // Remember to invalidate.
    }

    private int value2;
    public int Value2
    {
        get { return value2; }
        set { value2 = Math.Clamp(value, Minimum, Maximum); Invalidate(); }    // Remember to invalidate.
    }
}

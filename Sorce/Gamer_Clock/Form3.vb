Public Class Form3
    Private Sub Form3_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Me.Location = Screen.AllScreens(My.Settings.Main_Monitor - 1).Bounds.Location + New Point(Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Width / 2 - Me.Width / 2, Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Height / 2 - Me.Height / 2)
    End Sub
End Class
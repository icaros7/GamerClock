Public Class Form1

    Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
        Hour.Text = DateTime.Now.ToString("HH")
        Min.Text = DateTime.Now.ToString("mm")
        Sec.Text = DateTime.Now.ToString("ss")
    End Sub

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Hour.Text = DateTime.Now.ToString("HH")
        Min.Text = DateTime.Now.ToString("mm")
        Sec.Text = DateTime.Now.ToString("ss")
        For Each OneProcess As Process In Process.GetProcesses
            ListBox1.Items.Add(OneProcess.ProcessName + ".exe")
        Next
        Dim i As Long
        For i = 0 To 60 Step 1
            If i < 10 Then
                Dim pad As String
                pad = i.ToString("00")
                Mins.Items.Add(pad)
            Else
                Mins.Items.Add(i)
            End If
        Next
        For i = 0 To 60 Step 1
            If i < 10 Then
                Dim pad As String
                pad = i.ToString("00")
                Secs.Items.Add(pad)
            Else
                Secs.Items.Add(i)
            End If
        Next
        Hours.SelectedItem = DateTime.Now.ToString("HH")
        Mins.SelectedIndex = 0
        Secs.SelectedIndex = 0
        If Not ListBox1.FindString("RTSS.exe") <> -1 Then
            MsgBox("RivaTuner Statistics Server가 감지되지 않습니다." + vbCrLf + "프로그램을 종료합니다.", vbCritical, "오류")
            End
        End If
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If Processs.Text = "종료할 프로세스를 선택" Then
            MsgBox("종료할 프로세스를 선택해주세요!!", vbCritical, "오류")
        Else

            If MsgBox("선택한 프로세스 : " + Processs.Text + vbCrLf + "종료 예정 시간 : " + Hours.Text + "시 " + Mins.Text + "분 " + Secs.Text + "초" + vbCrLf + vbCrLf + "맞냐?", vbQuestion + vbYesNo, "확인") = vbYes Then
                Me.WindowState = FormWindowState.Minimized
            End If

        End If
    End Sub

    Private Sub NotifyIcon1_DoubleClick(ByVal sender As Object, ByVal e As System.EventArgs) Handles NotifyIcon1.DoubleClick
        ShowInTaskbar = True
        Me.WindowState = FormWindowState.Normal
        NotifyIcon1.Visible = False
    End Sub

    Private Sub LinkLabel1_LinkClicked(sender As Object, e As LinkLabelLinkClickedEventArgs) Handles CopyLeft.LinkClicked
        Shell("explorer http://minnote.net")
    End Sub

    Private Sub RadioButton4_CheckedChanged(sender As Object, e As EventArgs) Handles RadioButton4.CheckedChanged
        If RadioButton4.Checked = True Then
            GroupBox1.Enabled = True
        Else
            GroupBox1.Enabled = False
        End If
    End Sub

    Private Sub ListBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ListBox1.SelectedIndexChanged
        Processs.Text = ListBox1.SelectedItem
    End Sub

    Private Sub Form1_Resize(sender As Object, e As EventArgs) Handles Me.Resize
        If Me.WindowState = FormWindowState.Minimized Then
            NotifyIcon1.Visible = True
            NotifyIcon1.Icon = SystemIcons.Application
            NotifyIcon1.BalloonTipIcon = ToolTipIcon.Info
            NotifyIcon1.BalloonTipTitle = "Verificador corriendo"
            NotifyIcon1.BalloonTipText = "Verificador corriendo"
            NotifyIcon1.ShowBalloonTip(50000)
            ShowInTaskbar = False
        End If
    End Sub
End Class

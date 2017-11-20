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
        ListBox1_DoubleClick(sender, New System.EventArgs())
        Dim i As Long
        For i = 0 To 59 Step 1
            If i < 10 Then
                Dim pad As String
                pad = i.ToString("00")
                Mins.Items.Add(pad)
            Else
                Mins.Items.Add(i)
            End If
        Next
        Secs.Items.Add("00")
        If Not ListBox1.FindString("RTSS.exe") <> -1 Then
            MsgBox("RivaTuner Statistics Server가 감지되지 않습니다." + vbCrLf + "프로그램을 종료합니다.", vbCritical, "오류")
            End
        End If
        Hours.SelectedItem = DateTime.Now.ToString("HH")
        Mins.SelectedIndex = Val(DateTime.Now.ToString("mm"))
        Secs.SelectedIndex = 0

    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If Processs.Text = "종료할 프로세스를 선택" Or Processs.Text = "_더블클릭 시 새로고침_" Then
            MsgBox("종료할 프로세스를 선택해주세요!!", vbCritical, "오류")
        Else

            If MsgBox("선택한 프로세스 : " + Processs.Text + vbCrLf + "종료 예정 시간 : " + Hours.Text + "시 " + Mins.Text + "분 " + Secs.Text + "초" + vbCrLf + vbCrLf + "맞냐?", vbQuestion + vbYesNo, "확인") = vbYes Then
                Me.WindowState = FormWindowState.Minimized
                Timer2.Enabled = True
            End If

        End If
    End Sub

    Private Sub NotifyIcon1_DoubleClick(ByVal sender As Object, ByVal e As System.EventArgs) Handles NotifyIcon1.DoubleClick
        ShowInTaskbar = True
        Me.WindowState = FormWindowState.Normal
        NotifyIcon1.Visible = False
    End Sub

    Private Sub LinkLabel1_LinkClicked(sender As Object, e As LinkLabelLinkClickedEventArgs) Handles CopyLeft.LinkClicked
        Shell("explorer https://github.com/icaros7/GamerClock")
    End Sub

    Private Sub ListBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ListBox1.SelectedIndexChanged
        Processs.Text = ListBox1.SelectedItem
    End Sub

    Private Sub Form1_Resize(sender As Object, e As EventArgs) Handles Me.Resize
        If Me.WindowState = FormWindowState.Minimized Then
            NotifyIcon1.Visible = True
            NotifyIcon1.Icon = SystemIcons.Application
            NotifyIcon1.BalloonTipIcon = ToolTipIcon.Info
            NotifyIcon1.BalloonTipTitle = "Gamer Clock"
            NotifyIcon1.BalloonTipText = "Gamer Clock이 실행 중 입니다!" + vbCrLf + "종료 예정 시간 : " + Hours.Text + "시 " + Mins.Text + "분 " + Secs.Text + "초"
            NotifyIcon1.ShowBalloonTip(50000)
            ShowInTaskbar = False
        End If
    End Sub

    Private Sub Exit_Form_Click(sender As Object, e As EventArgs) Handles Exit_Form.Click
        If MsgBox("Gamer Clock 을 종료 하시겠습니까?" + vbCrLf + "시계 기능이 종료됩니다!!", vbQuestion + vbYesNo, "종료") = vbYes Then
            End
        End If
    End Sub

    Private Sub Open_Form_Click(sender As Object, e As EventArgs) Handles Open_Form.Click
        If MsgBox("알림 기능을 중지하고, 메인화면으로 돌아가시겠습니까?", vbQuestion + vbYesNo, "보이기") = vbYes Then
            Me.WindowState = FormWindowState.Normal
        End If
    End Sub

    Private Sub RadioButton1_Click(sender As Object, e As EventArgs) Handles RadioButton1.Click
        Label1_Click(sender, New System.EventArgs())
        Dim _30min As Int32
        Dim tmp As Int32
        Dim hr As Int32
        hr = 0
        _30min = Val(Date.Now.ToString("mm")) + 30
        If _30min > 59 Then
            Do
                tmp = _30min - 60
                hr += 1
            Loop Until (0 < tmp < 59)
            Hours.SelectedIndex = Hours.SelectedIndex + hr
            Mins.SelectedIndex = tmp
        Else
            Mins.SelectedIndex = _30min
        End If
        GroupBox1.Enabled = False
    End Sub

    Private Sub Label1_Click(sender As Object, e As EventArgs) Handles Label1.Click
        Hours.SelectedItem = DateTime.Now.ToString("HH")
        Mins.SelectedIndex = Val(DateTime.Now.ToString("mm"))
        Secs.SelectedIndex = 0
    End Sub

    Private Sub RadioButton2_Click(sender As Object, e As EventArgs) Handles RadioButton2.Click
        Label1_Click(sender, New System.EventArgs())
        Hours.SelectedIndex += 1
        GroupBox1.Enabled = False
    End Sub

    Private Sub RadioButton3_Click(sender As Object, e As EventArgs) Handles RadioButton3.Click
        Label1_Click(sender, New System.EventArgs())
        Hours.SelectedIndex += 2
        GroupBox1.Enabled = False
    End Sub

    Private Sub RadioButton4_Click(sender As Object, e As EventArgs) Handles RadioButton4.Click
        Label1_Click(sender, New System.EventArgs())
        GroupBox1.Enabled = True
    End Sub

    Private Sub Timer2_Tick(sender As Object, e As EventArgs) Handles Timer2.Tick
        If (Hour.Text = Hours.Text) And (Min.Text = Mins.Text) And (Sec.Text = Secs.Text) Then
            Me.WindowState = FormWindowState.Normal
            MsgBox("ok")
            Shell("taskkill -f -im " + Processs.Text, AppWinStyle.Hide)
            Timer2.Enabled = False
        End If
    End Sub

    Private Sub ListBox1_DoubleClick(sender As Object, e As EventArgs) Handles ListBox1.DoubleClick
        ListBox1.Items.Clear()
        ListBox2.Items.Clear()
        ListBox1.Items.Add("_더블클릭 시 새로고침_")
        ListBox2.Items.Add("_더블클릭 시 새로고침_")
        For Each OneProcess As Process In Process.GetProcesses
            ListBox1.Items.Add(OneProcess.ProcessName + ".exe")
            ListBox2.Items.Add(OneProcess.ProcessName + ".exe")
        Next
    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        If search.Text = "" Then
            ListBox1_DoubleClick(sender, New System.EventArgs())
        Else
            ListBox1.Items.Clear()
            ListBox1.Items.Add("_더블클릭 시 새로고침_")
            For i As Integer = 0 To ListBox2.Items.Count - 1
                If ListBox2.Items(i).ToLower.StartsWith(Me.search.Text.Trim.ToLower) Then
                    ListBox1.Items.Add(ListBox2.Items(i))
                End If
            Next
        End If
    End Sub

    Private Sub search_KeyDown(sender As Object, e As KeyEventArgs) Handles search.KeyDown
        If e.KeyCode = Keys.Return Then
            Button2_Click(sender, New System.EventArgs())
        End If
    End Sub
End Class

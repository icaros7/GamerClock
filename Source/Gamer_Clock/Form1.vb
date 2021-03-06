﻿Imports System.ComponentModel
Imports System.IO
Imports System.IO.File

Public Class Form1

    Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
        Hour.Text = DateTime.Now.ToString("HH")
        Min.Text = DateTime.Now.ToString("mm")
        Sec.Text = DateTime.Now.ToString("ss")
    End Sub

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        If My.Computer.FileSystem.FileExists(Application.StartupPath + "\Gamer_Clock_Update.exe") Then
            Shell("taskkill -f -im Gamer_Clock_Update.exe", AppWinStyle.Hide)
        End If
        If Not My.Computer.FileSystem.FileExists(Application.StartupPath + "\RTSS_Gamer_Clock.exe") Then
            On Error Resume Next
            Shell(TextBox2.Text)
        End If
        If UBound(Diagnostics.Process.GetProcessesByName(Diagnostics.Process.GetCurrentProcess.ProcessName)) > 0 Then
            MsgBox("Gamer Clock이 이미 실행중입니다!", vbExclamation, "오류")
            End
        End If
        Me.Location = Screen.AllScreens(My.Settings.Main_Monitor - 1).Bounds.Location + New Point(Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Width / 2 - Me.Width / 2, Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Height / 2 - Me.Height / 2)
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
                Secs.Items.Add(pad)
            Else
                Mins.Items.Add(i)
                Secs.Items.Add(i)
            End If
        Next
        'Secs.Items.Add("00")
        If Not ListBox1.FindString("RTSS.exe") <> -1 Then
            MsgBox("RivaTuner Statistics Server가 감지되지 않습니다.", vbInformation, "안내")
            RTSSHook.Checked = False
            RTSSHook.Enabled = False
        End If
        Hours.SelectedItem = DateTime.Now.ToString("HH")
        If DateTime.Now.ToString("mm") = 59 Then
            Mins.SelectedIndex = 0
            Hours.SelectedIndex += 1
        Else
            Mins.SelectedIndex = Val(DateTime.Now.ToString("mm")) + 1
        End If
        Secs.SelectedIndex = 0
        TextBox1.Text = My.Settings.Save_Msg
        If My.Settings.Auto_Search_config = True Then
            SearchBox.Text = My.Settings.Auto_Search
            Button2_Click(sender, New System.EventArgs())
        End If
        ListBox1.SelectedIndex = 0
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If Processs.Text = "종료할 프로세스를 선택" Or Processs.Text = "_더블클릭 시 새로고침_" Or Processs.Text = "종료할 프로세스를 입력" Then
            MsgBox("종료할 프로세스를 선택해주세요!!", vbCritical, "오류")
        Else

            If MsgBox("선택한 프로세스 : " + Processs.Text + vbCrLf + "종료 예정 시간 : " + Hours.Text + "시 " + Mins.Text + "분 " + Secs.Text + "초" + vbCrLf + "메시지 : " + TextBox1.Text + vbCrLf + vbCrLf + "모든 것이 정확한가요?", vbQuestion + vbYesNo, "확인") = vbYes Then
                Me.WindowState = FormWindowState.Minimized
                Timer2.Enabled = True

                If RTSSHook.Checked = True Then
                    '리바튜너
                    Re_OSD_Click(sender, New System.EventArgs())
                End If
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
        If ListBox1.SelectedItem = "_더블클릭 시 새로고침_" Then
            Processs.Text = "종료할 프로세스를 선택"
        Else
            Processs.Text = ListBox1.SelectedItem
        End If
    End Sub

    Private Sub Form1_Resize(sender As Object, e As EventArgs) Handles Me.Resize
        If Me.WindowState = FormWindowState.Minimized Then
            NotifyIcon1.Visible = True
            NotifyIcon1.Icon = Me.Icon
            NotifyIcon1.BalloonTipIcon = ToolTipIcon.Info
            NotifyIcon1.BalloonTipTitle = "Gamer Clock"
            NotifyIcon1.BalloonTipText = "Gamer Clock이 실행 중 입니다!" + vbCrLf + "종료 예정 시간 : " + Hours.Text + "시 " + Mins.Text + "분 " + Secs.Text + "초"
            NotifyIcon1.ShowBalloonTip(50000)
            ShowInTaskbar = False
        End If
    End Sub

    Private Sub Exit_Form_Click(sender As Object, e As EventArgs) Handles Exit_Form.Click
        If MsgBox("Gamer Clock 을 종료 하시겠습니까?" + vbCrLf + "시계 기능이 종료됩니다!!", vbQuestion + vbYesNo, "종료") = vbYes Then
            Shell("taskkill -f -im RTSS_Gamer_Clock.exe", AppWinStyle.Hide)
            End
        End If
    End Sub

    Private Sub Open_Form_Click(sender As Object, e As EventArgs) Handles Open_Form.Click
        If MsgBox("알림 기능을 중지하고, 메인화면으로 돌아가시겠습니까?", vbQuestion + vbYesNo, "보이기") = vbYes Then
            Me.WindowState = FormWindowState.Normal
            Timer2.Enabled = False
            Shell("taskkill -f -im RTSS_Gamer_Clock.exe", AppWinStyle.Hide)
        End If
    End Sub

    Private Sub RadioButton1_Click(sender As Object, e As EventArgs) Handles RadioButton1.Click
        Label1_Click(sender, New System.EventArgs())
        Dim _30min As Int32
        Dim tmp As Int32
        Dim hr As Int32 = 0
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
            Me.Show()
            If Processs.Text = "_윈도우 종료하기_" Then
                Shell("shutdown -s -f -t 5", AppWinStyle.Hide)
                MsgBox("5초 이내 윈도우가 종료됩니다." + vbCrLf + "사용해 주셔서 감사합니다." + vbCrLf + vbCrLf + "메시지 : " + TextBox1.Text, vbInformation, "Gamer Clock for OSD")
            Else
                Shell("taskkill -f -im " + Processs.Text, AppWinStyle.Hide)
                MsgBox("지정하신 시간이 다 되어 " + Processs.Text + "가 종료되었습니다." + vbCrLf + vbCrLf + "메시지 : " + TextBox1.Text, vbInformation, "안내")
            End If
            Shell("taskkill -f -im RTSS_Gamer_Clock.exe", AppWinStyle.Hide)
            System.IO.File.Delete(Application.StartupPath + "\OSD.ini")
            Timer2.Enabled = False
        End If
    End Sub

    Private Sub ListBox1_DoubleClick(sender As Object, e As EventArgs) Handles ListBox1.DoubleClick
        ListBox1.Items.Clear()
        ListBox2.Items.Clear()
        ListBox1.Items.Add("_더블클릭 시 새로고침_")
        ListBox1.Items.Add("_윈도우 종료하기_")
        For Each OneProcess As Process In Process.GetProcesses
            ListBox1.Items.Add(OneProcess.ProcessName + ".exe")
            ListBox2.Items.Add(OneProcess.ProcessName + ".exe")
        Next
    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        If SearchBox.Text = "" Then
            ListBox1_DoubleClick(sender, New System.EventArgs())
        Else
            ListBox1.Items.Clear()
            ListBox1.Items.Add("_더블클릭 시 새로고침_")
            For i As Integer = 0 To ListBox2.Items.Count - 1
                If ListBox2.Items(i).ToLower.StartsWith(Me.SearchBox.Text.Trim.ToLower) Then
                    ListBox1.Items.Add(ListBox2.Items(i))
                End If
            Next
        End If
    End Sub

    Private Sub search_KeyDown(sender As Object, e As KeyEventArgs) Handles SearchBox.KeyDown
        If e.KeyCode = Keys.Return Then
            Button2_Click(sender, New System.EventArgs())
        End If
    End Sub

    Private Sub Form1_Closing(sender As Object, e As CancelEventArgs) Handles Me.Closing
        End
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        Form2.ShowDialog()
    End Sub

    Private Sub Re_OSD_Click(sender As Object, e As EventArgs) Handles Re_OSD.Click
        '리바튜너
        Shell("taskkill -f -im RTSS_Gamer_Clock.exe", AppWinStyle.Hide)
        System.IO.File.Delete(Application.StartupPath + "\OSD.ini")
        Dim tmp As String
        If Processs.Text = "_윈도우 종료하기_" Then
            tmp = "Task=Shutdown Windows"
        Else
            tmp = "Task=kill " + Processs.Text
        End If
        Using writer As StreamWriter = New StreamWriter(Application.StartupPath + "\OSD.ini")
            writer.Write("[GamerClock]" & vbNewLine &
            "Set=" + Hours.Text + " : " + Mins.Text + " : " + Secs.Text & vbNewLine &
            tmp & vbNewLine &
            "Msg=" + TextBox1.Text
            )
        End Using
        'Shell(Application.StartupPath + "\RTSS_Gamer_Clock.exe", AppWinStyle.Hide) '관리자 권한 실행 요구
        Dim procStartInfo As New ProcessStartInfo
        Dim procExecuting As New Process
        With procStartInfo
            .UseShellExecute = True
            .FileName = Application.StartupPath + "\RTSS_Gamer_Clock.exe"
            .WindowStyle = ProcessWindowStyle.Minimized
            .Verb = "runas"
        End With

        procExecuting = Process.Start(procStartInfo)
    End Sub

    Private Sub search_TextChanged(sender As Object, e As EventArgs) Handles SearchBox.TextChanged
        Button2_Click(sender, New System.EventArgs())
    End Sub

    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        Form3.ShowDialog()
    End Sub

    Private Function CheckPro(ByVal input As String)
        Dim legth As Integer = input.Length - 1
        Dim i As Integer
        Dim cnt As Boolean = False
        For i = 0 To legth Step 1
            If input(i) = "*" Then
                If i = 0 Then
                    MsgBox("와일드카드(*)는 첫 문자로 올 수 없습니다!", vbCritical, "오류")
                    Return False
                Else
                    If cnt = True Then
                        MsgBox("와일드카드(*)는 하나만 사용할 수 있습니다!", vbCritical, "오류")
                        Return False
                    Else
                        cnt = True
                    End If
                End If
            End If
        Next
        If Not input(legth) = "*" Then
            MsgBox("와일드카드(*)는 끝 문자로만 올 수 있습니다!", vbCritical, "오류")
            Return False
        End If
        Return True
    End Function

    Private Sub UseWild_CheckedChanged(sender As Object, e As EventArgs) Handles UseWild.CheckedChanged
        If UseWild.Checked = True Then
            Processs.Text = "종료할 프로세스를 입력"
            Dim inputvalue As String
            Do
                inputvalue = InputBox("와일드카드(*)를 사용하여 여러개의 프로세스를 한번에 종료 할 수 있습니다." + vbCrLf + "와일드카드(*)는 정해지지 않은 문자 입니다." + vbCrLf + vbCrLf + "예를 들어 ""notepa*""를 입력시 아래와 같은 프로세스가 " + vbCrLf + "종료 됩니다." + vbCrLf + vbCrLf + "notepad.exe / notepad.jpg / notepaasdf.exe 등" + vbCrLf + """notepa""라는 이름으로 시작하는 모든 프로세스를 종료합니다.", "와일드카드(*) 사용", "")
                If inputvalue.Length < 2 Then
                    GoTo CancelCall
                End If
            Loop Until CheckPro(inputvalue) = True
            Processs.Text = inputvalue
            ListBox1.Enabled = False
            Label10.Enabled = False
            SearchBox.Enabled = False
            Button2.Enabled = False
        Else
CancelCall:
            UseWild.Checked = False
            Processs.Text = "종료할 프로세스를 선택"
            ListBox1.Enabled = True
            ListBox1_DoubleClick(sender, New System.EventArgs())
            ListBox1.SelectedIndex = 0
            Label10.Enabled = True
            SearchBox.Enabled = True
            Button2.Enabled = True
        End If
    End Sub
End Class

Imports System.IO
Imports System.Net
Imports System.Text
Imports System.Reflection

Public Class Form2
    Dim webReq As HttpWebRequest
    Dim webRes As HttpWebResponse
    Dim tr As TextReader
    Private Function getSource()
        On Error Resume Next
        webReq = WebRequest.Create("https://icaros7.github.io/index.html")
        webReq.Method = "GET"
        webRes = webReq.GetResponse

        If webRes.StatusCode = HttpStatusCode.OK Then

            tr = New StreamReader(webRes.GetResponseStream(), Encoding.GetEncoding(webRes.CharacterSet))

            Return tr.ReadToEnd
            webReq.Abort()
            tr.Close()
            Exit Function
        End If
        webReq.Abort()
    End Function

    Private Sub Form2_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Me.Location = Screen.AllScreens(My.Settings.Main_Monitor - 1).Bounds.Location + New Point(Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Width / 2 - Me.Width / 2, Screen.AllScreens(My.Settings.Main_Monitor - 1).WorkingArea.Height / 2 - Me.Height / 2)
        Dim Cnt As Int32 = Screen.AllScreens.Length
        Dim i As Int32 = 1
        Do Until i = Cnt + 1
            If i = 1 Then
                MonitorNum.Items.Add("1 (기본값)")
            Else
                MonitorNum.Items.Add(i)
            End If
            i += 1
        Loop
        MonitorNum.SelectedIndex = My.Settings.Main_Monitor - 1
        If My.Settings.MultiMonitor_config = True Then
            MultiMonitor_check.Checked = True
        Else
            MultiMonitor_check.Checked = False
        End If
        If My.Settings.Save_Msg_config = True Then
            Save_msg_check.Checked = True
        Else
            Save_msg_check.Checked = False
        End If
        If My.Settings.Auto_Search_config = True Then
            Auto_Search_check.Checked = True
        Else
            Auto_Search_check.Checked = False
        End If
        If My.Settings.Auto_Search_config = False Or My.Settings.Auto_Search = "" Then
            Label5.Text = "현재 저장된 프로세스는" + vbCrLf + "없습니다."
        Else
            Label5.Text = "현재 저장된 프로세스는" + vbCrLf + My.Settings.Auto_Search
        End If
    End Sub

    Private Sub Multimonitor_check_CheckedChanged(sender As Object, e As EventArgs) Handles MultiMonitor_check.CheckedChanged
        If MultiMonitor_check.Checked = True Then
            MonitorNum.Enabled = True
        Else
            MonitorNum.Enabled = False
        End If
    End Sub

    Private Sub Save_btn_Click(sender As Object, e As EventArgs) Handles Save_btn.Click
        If MultiMonitor_check.Checked = True Then
            My.Settings.MultiMonitor_config = True
            My.Settings.Main_Monitor = MonitorNum.SelectedIndex + 1
        Else
            My.Settings.MultiMonitor_config = False
            My.Settings.Main_Monitor = 1
        End If

        If Save_msg_check.Checked = False Or My.Settings.Save_Msg = "" Then
            My.Settings.Save_Msg_config = False
            My.Settings.Save_Msg = "Powered by hominlab@gmail.com"
        Else
            My.Settings.Save_Msg_config = True
        End If

        If Auto_Search_check.Checked = True Then
            My.Settings.Auto_Search_config = True
        Else
            My.Settings.Auto_Search_config = False
        End If

        My.Settings.Save()
        MsgBox("저장되었습니다!", vbInformation, "안내")
        Me.Hide()
    End Sub

    Private Sub Cancel_btn_Click(sender As Object, e As EventArgs) Handles Cancel_btn.Click
        Dim NoSave = MsgBox("저장 하지 않고 종료하시겠습니까?", vbExclamation + vbYesNoCancel, "취소")
        If NoSave = vbYes Then
            Me.Hide()
        ElseIf NoSave = vbNo Then
            Save_btn_Click(sender, New System.EventArgs())
        End If
    End Sub

    Private Sub LinkLabel1_LinkClicked(sender As Object, e As LinkLabelLinkClickedEventArgs) Handles Save_Msg_input.LinkClicked
        My.Settings.Save_Msg = InputBox("시작시 아래 메시지가 자동으로 입력됩니다." + vbCrLf + "빈칸인 경우 기본 값이 사용됩니다.", "메시지 저장", My.Settings.Save_Msg)
    End Sub

    Private Sub Auto_Search_msgbox_LinkClicked(sender As Object, e As LinkLabelLinkClickedEventArgs) Handles Auto_Search_input.LinkClicked
        My.Settings.Auto_Search = InputBox("시작시 자동으로 검색할 프로세스 명을 입력해 주세요." + vbCrLf + "빈칸인 경우 검색되지 않습니다.", "프로세스 설정", My.Settings.Auto_Search)
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim LastVersion As String
        LastVersion = Split(Split(getSource, "var GamerClockVersion = ")(1), ";")(0)
        LastVersion = Split(Split(LastVersion, """")(1), """")(0)
        Dim CurrentVersion As String = Application.ProductVersion
        If LastVersion = CurrentVersion Then
            MsgBox("최신버전의 Gamer Clock for OSD 를 실행중 입니다!", vbInformation, "안내")
        Else

            If MsgBox("새로운 업데이트가 있습니다. 설치 하시겠습니까?" + vbCrLf + vbCrLf + "설치된 버전 : " + CurrentVersion + vbCrLf + "최신 버전 : " + LastVersion, vbQuestion + vbYesNo, "업데이트") = vbYes Then
                On Error GoTo UpdateError
                Shell(TextBox1.Text)
                Shell(Application.StartupPath + "\Gamer_Clock_Update.bat", AppWinStyle.NormalFocus)
            Else
                MsgBox("가급적 최신 버전 사용을 권장합니다.", vbCritical, "안내")
            End If
        End If

UpdateError:
        If MsgBox("알 수 없는 오류입니다! 다시시도 하시겠습니까?", vbCritical + vbYesNo, "중대한 에러") = vbYes Then
            Button1_Click(sender, New System.EventArgs())
        End If
    End Sub
End Class
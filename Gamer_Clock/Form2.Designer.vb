<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Form2
    Inherits System.Windows.Forms.Form

    'Form은 Dispose를 재정의하여 구성 요소 목록을 정리합니다.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Windows Form 디자이너에 필요합니다.
    Private components As System.ComponentModel.IContainer

    '참고: 다음 프로시저는 Windows Form 디자이너에 필요합니다.
    '수정하려면 Windows Form 디자이너를 사용하십시오.  
    '코드 편집기에서는 수정하지 마세요.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.MultiMonitor_check = New System.Windows.Forms.CheckBox()
        Me.MonitorNum = New System.Windows.Forms.ComboBox()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.Cancel_btn = New System.Windows.Forms.Button()
        Me.Save_btn = New System.Windows.Forms.Button()
        Me.Save_msg_check = New System.Windows.Forms.CheckBox()
        Me.Save_Msg_input = New System.Windows.Forms.LinkLabel()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Auto_Search_check = New System.Windows.Forms.CheckBox()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.Auto_Search_input = New System.Windows.Forms.LinkLabel()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.TextBox1 = New System.Windows.Forms.TextBox()
        Me.SuspendLayout()
        '
        'MultiMonitor_check
        '
        Me.MultiMonitor_check.AutoSize = True
        Me.MultiMonitor_check.Location = New System.Drawing.Point(12, 12)
        Me.MultiMonitor_check.Name = "MultiMonitor_check"
        Me.MultiMonitor_check.Size = New System.Drawing.Size(128, 16)
        Me.MultiMonitor_check.TabIndex = 0
        Me.MultiMonitor_check.Text = "다중 모니터 설정 : "
        Me.MultiMonitor_check.UseVisualStyleBackColor = True
        '
        'MonitorNum
        '
        Me.MonitorNum.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.MonitorNum.Enabled = False
        Me.MonitorNum.FormattingEnabled = True
        Me.MonitorNum.Location = New System.Drawing.Point(146, 8)
        Me.MonitorNum.Name = "MonitorNum"
        Me.MonitorNum.Size = New System.Drawing.Size(126, 20)
        Me.MonitorNum.TabIndex = 1
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(10, 31)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(249, 24)
        Me.Label1.TabIndex = 2
        Me.Label1.Text = "여러개의 모니터를 사용하는 경우 프로그램과" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "OSD가 실행될 모니터를 설정 합니다."
        '
        'Cancel_btn
        '
        Me.Cancel_btn.Location = New System.Drawing.Point(197, 259)
        Me.Cancel_btn.Name = "Cancel_btn"
        Me.Cancel_btn.Size = New System.Drawing.Size(75, 23)
        Me.Cancel_btn.TabIndex = 3
        Me.Cancel_btn.Text = "취소"
        Me.Cancel_btn.UseVisualStyleBackColor = True
        '
        'Save_btn
        '
        Me.Save_btn.Location = New System.Drawing.Point(116, 259)
        Me.Save_btn.Name = "Save_btn"
        Me.Save_btn.Size = New System.Drawing.Size(75, 23)
        Me.Save_btn.TabIndex = 3
        Me.Save_btn.Text = "확인"
        Me.Save_btn.UseVisualStyleBackColor = True
        '
        'Save_msg_check
        '
        Me.Save_msg_check.AutoSize = True
        Me.Save_msg_check.Location = New System.Drawing.Point(12, 72)
        Me.Save_msg_check.Name = "Save_msg_check"
        Me.Save_msg_check.Size = New System.Drawing.Size(88, 16)
        Me.Save_msg_check.TabIndex = 5
        Me.Save_msg_check.Text = "메시지 저장"
        Me.Save_msg_check.UseVisualStyleBackColor = True
        '
        'Save_Msg_input
        '
        Me.Save_Msg_input.AutoSize = True
        Me.Save_Msg_input.Location = New System.Drawing.Point(10, 110)
        Me.Save_Msg_input.Name = "Save_Msg_input"
        Me.Save_Msg_input.Size = New System.Drawing.Size(185, 12)
        Me.Save_Msg_input.TabIndex = 7
        Me.Save_Msg_input.TabStop = True
        Me.Save_Msg_input.Text = "설정 하시려면 여기를 눌러주세요"
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Location = New System.Drawing.Point(10, 91)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(241, 12)
        Me.Label3.TabIndex = 8
        Me.Label3.Text = "시작시 메세지 내용을 자동으로 입력합니다."
        '
        'Auto_Search_check
        '
        Me.Auto_Search_check.AutoSize = True
        Me.Auto_Search_check.Location = New System.Drawing.Point(12, 140)
        Me.Auto_Search_check.Name = "Auto_Search_check"
        Me.Auto_Search_check.Size = New System.Drawing.Size(192, 16)
        Me.Auto_Search_check.TabIndex = 9
        Me.Auto_Search_check.Text = "시작시 자동으로 프로세스 검색"
        Me.Auto_Search_check.UseVisualStyleBackColor = True
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(10, 159)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(209, 24)
        Me.Label4.TabIndex = 10
        Me.Label4.Text = "시작시 자동으로 저장된 프로세스만을" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "리스트에 보여줍니다."
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Location = New System.Drawing.Point(10, 193)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(133, 24)
        Me.Label5.TabIndex = 11
        Me.Label5.Text = "현재 저장된 프로세스는" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10)
        '
        'Auto_Search_input
        '
        Me.Auto_Search_input.AutoSize = True
        Me.Auto_Search_input.Location = New System.Drawing.Point(10, 217)
        Me.Auto_Search_input.Name = "Auto_Search_input"
        Me.Auto_Search_input.Size = New System.Drawing.Size(185, 12)
        Me.Auto_Search_input.TabIndex = 12
        Me.Auto_Search_input.TabStop = True
        Me.Auto_Search_input.Text = "설정 하시려면 여기를 눌러주세요"
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(10, 244)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(193, 12)
        Me.Label2.TabIndex = 13
        Me.Label2.Text = "버그 제보 : hominlab@gmail.com"
        '
        'Button1
        '
        Me.Button1.Location = New System.Drawing.Point(12, 259)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(77, 23)
        Me.Button1.TabIndex = 14
        Me.Button1.Text = "업데이트"
        Me.Button1.UseVisualStyleBackColor = True
        '
        'TextBox1
        '
        Me.TextBox1.Location = New System.Drawing.Point(379, 79)
        Me.TextBox1.Name = "TextBox1"
        Me.TextBox1.Size = New System.Drawing.Size(61, 21)
        Me.TextBox1.TabIndex = 15
        Me.TextBox1.Text = "powershell ""(new-object Net.WebClient).DownloadFile('https://icaros7.github.io/Ga" &
    "merClock/Update.bat', './Gamer_Clock_Update.bat')"""
        '
        'Form2
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(7.0!, 12.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(280, 290)
        Me.Controls.Add(Me.TextBox1)
        Me.Controls.Add(Me.Button1)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.Auto_Search_input)
        Me.Controls.Add(Me.Label5)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.Auto_Search_check)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.Save_Msg_input)
        Me.Controls.Add(Me.Save_msg_check)
        Me.Controls.Add(Me.Save_btn)
        Me.Controls.Add(Me.Cancel_btn)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.MonitorNum)
        Me.Controls.Add(Me.MultiMonitor_check)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow
        Me.Name = "Form2"
        Me.StartPosition = System.Windows.Forms.FormStartPosition.Manual
        Me.Text = "Gamer Clock for OSD 옵션"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents MultiMonitor_check As CheckBox
    Friend WithEvents MonitorNum As ComboBox
    Friend WithEvents Label1 As Label
    Friend WithEvents Cancel_btn As Button
    Friend WithEvents Save_btn As Button
    Friend WithEvents Save_msg_check As CheckBox
    Friend WithEvents Save_Msg_input As LinkLabel
    Friend WithEvents Label3 As Label
    Friend WithEvents Auto_Search_check As CheckBox
    Friend WithEvents Label4 As Label
    Friend WithEvents Label5 As Label
    Friend WithEvents Auto_Search_input As LinkLabel
    Friend WithEvents Label2 As Label
    Friend WithEvents Button1 As Button
    Friend WithEvents TextBox1 As TextBox
End Class

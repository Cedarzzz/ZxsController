.386
.MODEL FLAT, C

.data

.CODE

CopyScreenData PROC     ; SourceData, DestinationData, BufferLength
	push ebp
	mov  ebp, esp
	sub	 esp, 20h		;��ջ

	mov	 ebx, dword ptr [ebp + 10h] ; BufferLength
	mov  esi, dword ptr [ebp + 0Ch] ; DeatinationData
	jmp  CopyEnd

CopyNextBlock:
									;[λ��(DWORD)][��ͬ���ݵĳ���(DWORD)][��ͬ������]
	mov edi, [ebp + 8h]				;SourceData
	lodsd							;lodsdָ���esiָ����ڴ�λ��4���ֽ����ݷ���eax�У�����esi����4���ֽ�
									;��[λ��(DWORD)] ����eax

	add edi, eax					;SourceDataƫ��eax
	lodsd							;��[��ͬ���ݵĳ���(DWORD)] ����eax
	mov ecx, eax					;��Ҫ���������ݵĳ��ȷ���ecx
	sub ebx, 8						;ebx(BufferLength)��ȥ����DWORD
	sub ebx, ecx					;ebx(BufferLength)��ȥ[��ͬ������]�ĳ���
	rep movsb						;movsb����һ���ֽڵ����ݣ�ͨ��SI��DI�������Ĵ��������ַ�����Դ��ַ��Ŀ���ַ
									;��DS:SIָ��ĵ�ַ�����ݸ��ǵ�SE:DI��,��SI��DI�Զ��ƶ�����һ��Ҫ���Ƶĵ�ַ
									;rep movb���ظ�movsb,�ظ����������Ӵ����ȣ������CX�Ĵ���

CopyEnd:
	cmp ebx, 0                      ;�Ƿ�д����
	jnz CopyNextBlock

	add esp, 20h					;�ָ���ջ
	pop ebp
	ret

CopyScreenData ENDP

END
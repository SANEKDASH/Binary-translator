#ifndef INSTRUCTION_ENCODING_HEADER
#define INSTRUCTION_ENCODING_HEADER

BackendErrs_t EncodeLeave(BackendContext *backend_context);

BackendErrs_t EncodeJump(BackendContext  *backend_context,
                         LogicalOpcode_t  logical_opcode,
                         Opcode_t         op_code,
                         int32_t          label_identifier);

BackendErrs_t EncodeRegisterAndRegister(BackendContext *backend_context,
                                        RegisterCode_t  dest_reg,
                                        RegisterCode_t  src_reg);

BackendErrs_t EncodeRegisterOrRegister(BackendContext *backend_context,
                                       RegisterCode_t  dest_reg,
                                       RegisterCode_t  src_reg);


BackendErrs_t EncodeCmpRegisterWithRegister(BackendContext *backend_context,
                                            RegisterCode_t  dest_reg,
                                            RegisterCode_t  src_reg);

BackendErrs_t EncodeCmpRegisterWithImmediate(BackendContext *backend_context,
                                             RegisterCode_t  dest_reg,
                                             ImmediateType_t immediate);

BackendErrs_t EncodeImulRegister(BackendContext *backend_context,
                                 RegisterCode_t  dest_reg);

BackendErrs_t EncodeDivRegister(BackendContext *backend_context,
                                RegisterCode_t  dest_reg);

BackendErrs_t EncodeXorRegisterWithRegister(BackendContext *backend_context,
                                            RegisterCode_t  dest_reg,
                                            RegisterCode_t  src_reg);

BackendErrs_t EncodeSubImmediateFromRegister(BackendContext *backend_context,
                                             RegisterCode_t  dest_reg,
                                             ImmediateType_t immediate);

BackendErrs_t EncodeSubRegisterFromRegister(BackendContext *backend_context,
                                            RegisterCode_t  src_reg,
                                            RegisterCode_t  dest_reg);

BackendErrs_t EncodeMovRegisterMemoryToRegister(BackendContext     *backend_context,
                                                RegisterCode_t      src_reg,
                                                RegisterCode_t      dest_reg,
                                                DisplacementType_t  displacement);

BackendErrs_t EncodeAddRegisterToRegister(BackendContext *backend_context,
                                          RegisterCode_t  src_reg,
                                          RegisterCode_t  dest_reg);

BackendErrs_t EncodeAddImmediateToRegister(BackendContext  *backend_context,
                                           ImmediateType_t  immediate,
                                           RegisterCode_t   dest_reg);

BackendErrs_t EncodeMovRegisterToRegisterMemory(BackendContext     *backend_context,
                                                RegisterCode_t      src_reg,
                                                RegisterCode_t      dest_reg,
                                                DisplacementType_t  displacement);

BackendErrs_t EncodeMovImmediateToRegister(BackendContext  *backend_context,
                                           ImmediateType_t  immediate,
                                           RegisterCode_t   dest_reg);

BackendErrs_t EncodePopInRegister(BackendContext *backend_context,
                                  RegisterCode_t  dest_reg);

BackendErrs_t EncodeRet(BackendContext *backend_context);

BackendErrs_t EncodeMovRegisterToRegister(BackendContext *backend_context,
                                          RegisterCode_t  src_reg,
                                          RegisterCode_t  dest_reg);

BackendErrs_t EncodePushRegister(BackendContext *backend_context,
                                 RegisterCode_t  reg);

BackendErrs_t EncodeCall(BackendContext  *backend_context,
                         LanguageContext *language_context,
                         size_t           func_pos);

#endif

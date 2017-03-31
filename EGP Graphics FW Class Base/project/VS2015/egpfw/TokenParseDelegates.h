#pragma once
#include <string>

class IToken;
class TokenParser;
class TokenStream;
class EmitableToken;

struct emitter_delegate;

typedef void(token_delegate_t)(const emitter_delegate&, TokenStream&, EmitableToken*);

std::string getEmissionString(IToken* t);

unsigned getLaneToken(TokenStream& in);
void emitUntilPunctuation(const emitter_delegate& out, TokenStream& in, std::string punc, bool emitPunc);

void parse_VertexIn100(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_VertexIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_VertexIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_VertexOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_VertexOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_VertexOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_FragmentIn100(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_FragmentIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_FragmentIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_FragmentOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_FragmentOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_FragmentOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_GeometryIn320(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_GeometryIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_GeometryOut320(const emitter_delegate& out, TokenStream& in, EmitableToken* t);
void parse_GeometryOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

void parse_Uniform(const emitter_delegate& out, TokenStream& in, EmitableToken* t);

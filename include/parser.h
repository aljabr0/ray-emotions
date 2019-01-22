#ifndef PARSER_H_
#define PARSER_H_

#include "internal.h"

struct rayem_tokens{
	GStringChunk *chunk;
	GPtrArray *tokens;
};

void rayem_tokens_destroy(struct rayem_tokens *t);
void rayem_tokens_reset(struct rayem_tokens *t);
void rayem_tokens_init(struct rayem_tokens *t);

int rayem_parser_read_single_line(FILE *input,GString *str,
		int max_str_len,
		int *eos);
int rayem_parser_get_tokens(GString *str,struct rayem_tokens *t);

#define RAYEM_TYPE_TOKEN                  (rayem_token_get_type())
#define RAYEM_TOKEN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TOKEN,RayemToken))
#define RAYEM_IS_TOKEN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TOKEN))
#define RAYEM_TOKEN_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TOKEN,RayemTokenClass))
#define RAYEM_IS_TOKEN_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TOKEN))
#define RAYEM_TOKEN_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TOKEN,RayemTokenClass))

struct _RayemToken{
	GObject parent_instance;
};

struct _RayemTokenClass{
	GObjectClass parent_class;
};

GType rayem_token_get_type(void);


#define RAYEM_TYPE_STRING_TOKEN                  (rayem_string_token_get_type())
#define RAYEM_STRING_TOKEN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_STRING_TOKEN,RayemStringToken))
#define RAYEM_IS_STRING_TOKEN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_STRING_TOKEN))
#define RAYEM_STRING_TOKEN_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_STRING_TOKEN,RayemStringTokenClass))
#define RAYEM_IS_STRING_TOKEN_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_STRING_TOKEN))
#define RAYEM_STRING_TOKEN_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_STRING_TOKEN,RayemStringTokenClass))

struct _RayemStringToken{
	RayemToken parent_instance;
	char *value;
};

struct _RayemStringTokenClass{
	RayemTokenClass parent_class;
};

GType rayem_string_token_get_type(void);
RayemStringToken *rayem_string_token_new(char *value);


#define RAYEM_TYPE_SYMBOL_TOKEN                  (rayem_symbol_token_get_type())
#define RAYEM_SYMBOL_TOKEN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_SYMBOL_TOKEN,RayemSymbolToken))
#define RAYEM_IS_SYMBOL_TOKEN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_SYMBOL_TOKEN))
#define RAYEM_SYMBOL_TOKEN_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_SYMBOL_TOKEN,RayemSymbolTokenClass))
#define RAYEM_IS_SYMBOL_TOKEN_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_SYMBOL_TOKEN))
#define RAYEM_SYMBOL_TOKEN_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_SYMBOL_TOKEN,RayemSymbolTokenClass))

struct _RayemSymbolToken{
	RayemToken parent_instance;
	char *value;
};

struct _RayemSymbolTokenClass{
	RayemTokenClass parent_class;
};

GType rayem_symbol_token_get_type(void);
RayemSymbolToken *rayem_symbol_token_new(char *value);


#define RAYEM_TYPE_NUMBER_TOKEN                  (rayem_number_token_get_type())
#define RAYEM_NUMBER_TOKEN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_NUMBER_TOKEN,RayemNumberToken))
#define RAYEM_IS_NUMBER_TOKEN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_NUMBER_TOKEN))
#define RAYEM_NUMBER_TOKEN_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_NUMBER_TOKEN,RayemNumberTokenClass))
#define RAYEM_IS_NUMBER_TOKEN_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_NUMBER_TOKEN))
#define RAYEM_NUMBER_TOKEN_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_NUMBER_TOKEN,RayemNumberTokenClass))

struct _RayemNumberToken{
	RayemToken parent_instance;
	double value;
};

struct _RayemNumberTokenClass{
	RayemTokenClass parent_class;
};

GType rayem_number_token_get_type(void);
RayemNumberToken *rayem_number_token_new(double value);

#endif /* PARSER_H_ */

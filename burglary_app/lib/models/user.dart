class User {
  final int id;
  final String email;
  final String? fullName;
  final String? phoneNumber;
  final bool isActive;
  final String role;
  final DateTime createdAt;
  
  User({
    required this.id,
    required this.email,
    this.fullName,
    this.phoneNumber,
    required this.isActive,
    required this.role,
    required this.createdAt,
  });
  
  factory User.fromJson(Map<String, dynamic> json) {
    return User(
      id: json['id'] as int,
      email: json['email'] as String,
      fullName: json['full_name'] as String?,
      phoneNumber: json['phone_number'] as String?,
      isActive: json['is_active'] as bool,
      role: json['role'] as String,
      createdAt: DateTime.parse(json['created_at'] as String),
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'email': email,
      'full_name': fullName,
      'phone_number': phoneNumber,
      'is_active': isActive,
      'role': role,
      'created_at': createdAt.toIso8601String(),
    };
  }
}

class UserLogin {
  final String email;
  final String password;
  
  UserLogin({required this.email, required this.password});
  
  Map<String, dynamic> toJson() {
    return {
      'email': email,
      'password': password,
    };
  }
}

class UserCreate {
  final String email;
  final String password;
  final String? fullName;
  final String? phoneNumber;
  
  UserCreate({
    required this.email,
    required this.password,
    this.fullName,
    this.phoneNumber,
  });
  
  Map<String, dynamic> toJson() {
    return {
      'email': email,
      'password': password,
      'full_name': fullName,
      'phone_number': phoneNumber,
    };
  }
}

class Token {
  final String accessToken;
  final String tokenType;
  
  Token({
    required this.accessToken,
    this.tokenType = 'bearer',
  });
  
  factory Token.fromJson(Map<String, dynamic> json) {
    return Token(
      accessToken: json['access_token'] as String,
      tokenType: json['token_type'] as String? ?? 'bearer',
    );
  }
}


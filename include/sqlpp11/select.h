/*
 * Copyright (c) 2013, Roland Bock
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLPP_SELECT_H
#define SQLPP_SELECT_H

#include <sqlpp11/result_row.h>
#include <sqlpp11/result.h>
#include <sqlpp11/select_fwd.h>
#include <sqlpp11/noop.h>
#include <sqlpp11/select_flag_list.h>
#include <sqlpp11/select_expression_list.h>
#include <sqlpp11/select_pseudo_table.h>
#include <sqlpp11/from.h>
#include <sqlpp11/where.h>
#include <sqlpp11/group_by.h>
#include <sqlpp11/having.h>
#include <sqlpp11/order_by.h>
#include <sqlpp11/limit.h>
#include <sqlpp11/offset.h>
#include <sqlpp11/expression.h>
#include <sqlpp11/field.h>

#include <sqlpp11/detail/wrong.h>
#include <sqlpp11/detail/make_flag_tuple.h>
#include <sqlpp11/detail/make_expression_tuple.h>

#include <sstream>

namespace sqlpp
{
	template<
		typename Database,
		typename Flags,
		typename... NamedExpr,
		typename From,
		typename Where,
		typename GroupBy,
		typename Having,
		typename OrderBy,
		typename Limit,
		typename Offset
		>
		struct select_t<Database, Flags, select_expression_list_t<std::tuple<NamedExpr...>>, From, Where, GroupBy, Having, OrderBy, Limit, Offset>
		: public select_expression_list_t<std::tuple<NamedExpr...>>::_value_type::template operators<select_t<
		                 Database,
										 Flags, 
										 select_expression_list_t<std::tuple<NamedExpr...>>, 
										 From, 
										 Where, 
										 GroupBy, 
										 Having, 
										 OrderBy, 
										 Limit, 
										 Offset>>
		{
			using _Database = Database;
			using _From = From;
			using ExpressionList = select_expression_list_t<std::tuple<NamedExpr...>>;

			static_assert(is_noop<Flags>::value or is_select_flag_list_t<Flags>::value, "invalid list of select flags");
			static_assert(is_select_expression_list_t<ExpressionList>::value, "invalid list of select expressions");
			static_assert(is_noop<From>::value or is_from_t<From>::value, "invalid 'from' argument");
			static_assert(is_noop<Where>::value or is_where_t<Where>::value, "invalid 'where' argument");
			static_assert(is_noop<GroupBy>::value or is_group_by_t<GroupBy>::value, "invalid 'group by' arguments");
			static_assert(is_noop<Having>::value or is_having_t<Having>::value, "invalid 'having' arguments");
			static_assert(is_noop<OrderBy>::value or is_order_by_t<OrderBy>::value, "invalid 'order by' arguments");
			static_assert(is_noop<Limit>::value or is_limit_t<Limit>::value, "invalid 'limit' arguments");
			static_assert(is_noop<Offset>::value or is_offset_t<Offset>::value, "invalid 'offset' arguments");

			using _is_select = tag_yes;
			using _requires_braces = tag_yes;

			template<typename FromT> 
				using set_from_t = select_t<Database, Flags, ExpressionList, FromT, Where, GroupBy, Having, OrderBy, Limit, Offset>;
			template<typename WhereT>
				using set_where_t = select_t<Database, Flags, ExpressionList, From, WhereT, GroupBy, Having, OrderBy, Limit, Offset>;
			template<typename GroupByT>
				using set_group_by_t = select_t<Database, Flags, ExpressionList, From, Where, GroupByT, Having, OrderBy, Limit, Offset>;
			template<typename HavingT>
				using set_having_t = select_t<Database, Flags, ExpressionList, From, Where, GroupBy, HavingT, OrderBy, Limit, Offset>;
			template<typename OrderByT>
				using set_order_by_t = select_t<Database, Flags, ExpressionList, From, Where, GroupBy, Having, OrderByT, Limit, Offset>;
			template<typename LimitT>
				using set_limit_t = select_t<Database, Flags, ExpressionList, From, Where, GroupBy, Having, OrderBy, LimitT, Offset>;
			template<typename OffsetT>
			using set_offset_t = select_t<Database, Flags, ExpressionList, From, Where, GroupBy, Having, OrderBy, Limit, OffsetT>;

			using _result_row_t = result_row_t<make_field_t<NamedExpr>...>;

			// Indicators
			using _value_type = typename std::conditional<
				is_noop<From>::value, 
				no_value_t, // If there is no from, the select is not complete (this logic is a bit simple, but better than nothing)
				typename ExpressionList::_value_type>::type;

			using _name_t = typename ExpressionList::_name_t;

			// The standard constructors, assigment operators and destructor
			select_t(Flags&& flags, ExpressionList&& expression_list):
				_flags(std::move(flags)),
				_expression_list(std::move(expression_list))
			{
				static_assert(std::is_same<select_t, sqlpp::select_t<Database, Flags, ExpressionList>>::value,
						"basic constructor only available for select_t<Flags, ExpressionList> (default template parameters)");
			}

			select_t(const select_t& rhs) = default;
			select_t(select_t&& rhs) = default;
			select_t& operator=(const select_t& rhs) = default;
			select_t& operator=(select_t&& rhs) = default;
			~select_t() = default;

			// Other constructors

			constexpr select_t(Flags&& flags, ExpressionList&& expression_list, From&& from,
					Where&& where, GroupBy&& group_by, Having&& having,
					OrderBy&& order_by, Limit&& limit, Offset&& offset):
				_flags(std::move(flags)),
				_expression_list(std::move(expression_list)),
				_from(std::move(from)),
				_where(std::move(where)),
				_group_by(std::move(group_by)),
				_having(std::move(having)),
				_order_by(std::move(order_by)),
				_limit(std::move(limit)),
				_offset(std::move(offset))
			{
			}

			constexpr select_t(const Flags& flags, const ExpressionList& expression_list, const From& from,
					const Where& where, const GroupBy& group_by, const Having& having,
					const OrderBy& order_by, const Limit& limit, const Offset& offset):
				_flags(flags),
				_expression_list(expression_list),
				_from(from),
				_where(where),
				_group_by(group_by),
				_having(having),
				_order_by(order_by),
				_limit(limit),
				_offset(offset)
			{
			}

			// sqlpp functions
			template<typename... Table>
				auto from(Table&&... table)
				-> set_from_t<from_t<typename std::decay<Table>::type...>>
				{
					static_assert(not is_noop<ExpressionList>::value, "cannot call from() without having selected anything");
					static_assert(is_noop<From>::value, "cannot call from() twice for a single select");
					return {
							_flags, 
							_expression_list, 
							{std::tuple<typename std::decay<Table>::type...>{std::forward<Table>(table)...}}, 
							_where, 
							_group_by, 
							_having, 
							_order_by, 
							_limit,
							_offset
							};
				}

				auto dynamic_from()
				-> set_from_t<dynamic_from_t<Database>>
				{
					static_assert(not std::is_same<Database, void>::value, "cannot call dynamic_from() in a non-dynamic select");
					static_assert(not is_noop<ExpressionList>::value, "cannot call from() without having selected anything");
					static_assert(is_noop<From>::value, "cannot call from() twice for a single select");
					return {
							_flags, 
							_expression_list, 
							{{}}, 
							_where, 
							_group_by, 
							_having, 
							_order_by, 
							_limit,
							_offset
							};
				}

			template<typename Table>
				void add_from(Table&& table)
				{
					static_assert(not is_noop<ExpressionList>::value, "cannot call add_from() without having selected anything");
					static_assert(is_dynamic_t<From>::value, "cannot call add_from() in a non-dynamic from");

					_from.add(std::forward<Table>(table));
				}

			template<typename Expr>
				auto where(Expr&& expr)
				-> set_where_t<where_t<typename std::decay<Expr>::type>> 
				{
					static_assert(not is_noop<From>::value, "cannot call where() without a from()");
					static_assert(is_noop<Where>::value, "cannot call where() or dynamic_where() twice for a single select");
					return {
							_flags, 
							_expression_list, 
							_from, 
							{std::forward<Expr>(expr)}, 
							_group_by,
							_having,
							_order_by,
							_limit,
							_offset,
							};
				}

				auto dynamic_where()
				-> set_where_t<dynamic_where_t<Database>>
				{
					static_assert(not is_noop<From>::value, "cannot call dynamic_where() without a from()");
					static_assert(is_noop<Where>::value, "cannot call where() or dynamic_where() twice for a single select");
					return {
							_flags, 
							_expression_list, 
							_from, 
							{{}}, 
							_group_by,
							_having,
							_order_by,
							_limit,
							_offset,
							};
				}

			template<typename Expr>
				void add_where(Expr&& expr)
				{
					static_assert(is_dynamic_t<Where>::value, "cannot call add_where() in a non-dynamic where");

					_where.add(std::forward<Expr>(expr));
				}

			template<typename... Col>
				auto group_by(Col&&... column)
				-> set_group_by_t<group_by_t<typename std::decay<Col>::type...>>
				{
					static_assert(not is_noop<From>::value, "cannot call group_by() without a from()");
					static_assert(is_noop<GroupBy>::value, "cannot call group_by() twice for a single select");
					return {
							_flags, 
							_expression_list,
							_from,
							_where,
							{std::tuple<typename std::decay<Col>::type...>{std::forward<Col>(column)...}},
							_having,
							_order_by,
							_limit,
							_offset,
							};
				}

				auto group_by()
				-> set_group_by_t<group_by_t<Database>>
				{
					static_assert(not is_noop<From>::value, "cannot call group_by() without a from()");
					static_assert(is_noop<GroupBy>::value, "cannot call group_by() twice for a single select");
					return {
							_flags, 
							_expression_list,
							_from,
							_where,
							{{}},
							_having,
							_order_by,
							_limit,
							_offset,
							};
				}

			template<typename Expr>
				void add_group_by(Expr&& expr)
				{
					static_assert(is_dynamic_t<GroupBy>::value, "cannot call add_group_by() in a non-dynamic group_by");

					_group_by.add(std::forward<Expr>(expr));
				}

			template<typename Expr>
				auto having(Expr&& expr)
				-> set_having_t<having_t<typename std::decay<Expr>::type>> 
				{
					static_assert(not is_noop<GroupBy>::value, "cannot call having() without a group_by");
					static_assert(is_noop<Having>::value, "cannot call having() twice for a single select");
					return {
							_flags, 
							_expression_list,
							_from,
							_where,
							_group_by,
							{std::forward<Expr>(expr)},
							_order_by,
							_limit,
							_offset,
							};
				}

			auto dynamic_having()
				-> set_having_t<dynamic_having_t<Database>>
				{
					static_assert(not is_noop<GroupBy>::value, "cannot call having() without a group_by");
					static_assert(is_noop<Having>::value, "cannot call having() twice for a single select");
					return {
						_flags, 
							_expression_list,
							_from,
							_where,
							_group_by,
							{{}},
							_order_by,
							_limit,
							_offset,
					};
				}

			template<typename Expr>
				void add_having(Expr&& expr)
				{
					static_assert(is_dynamic_t<Having>::value, "cannot call add_having() in a non-dynamic having");

					_having.add(std::forward<Expr>(expr));
				}

			template<typename... OrderExpr>
				auto order_by(OrderExpr&&... expr)
				-> set_order_by_t<order_by_t<typename std::decay<OrderExpr>::type...>>
				{
					static_assert(not is_noop<From>::value, "cannot call order_by() without a from()");
					static_assert(is_noop<OrderBy>::value, "cannot call order_by() twice for a single select");
					return {
							_flags, 
							_expression_list,
							_from,
							_where,
							_group_by,
							_having,
							{std::tuple<typename std::decay<OrderExpr>::type...>{std::forward<OrderExpr>(expr)...}},
							_limit,
							_offset,
							};
				}

			auto dynamic_order_by()
				-> set_order_by_t<dynamic_order_by_t<Database>>
				{
					static_assert(not is_noop<From>::value, "cannot call order_by() without a from()");
					static_assert(is_noop<OrderBy>::value, "cannot call order_by() twice for a single select");
					return {
							_flags, 
							_expression_list,
							_from,
							_where,
							_group_by,
							_having,
							{{}},
							_limit,
							_offset,
							};
				}

			template<typename Expr>
				void add_order_by(Expr&& expr)
				{
					static_assert(is_dynamic_t<OrderBy>::value, "cannot call add_order_by() in a non-dynamic order_by");

					_order_by.add(std::forward<Expr>(expr));
				}

			auto limit(std::size_t limit)
				-> set_limit_t<limit_t>
			{
				static_assert(not is_noop<From>::value, "cannot call limit() without a from()");
				static_assert(is_noop<Limit>::value, "cannot call limit() twice for a single select");
				return {
						_flags, 
						_expression_list,
						_from,
						_where,
						_group_by,
						_having,
						_order_by,
						{limit},
						_offset,
				};
			}

			auto dynamic_limit()
				->set_limit_t<dynamic_limit_t>
			{
				static_assert(not is_noop<From>::value, "cannot call limit() without a from()");
				static_assert(is_noop<Limit>::value, "cannot call limit() twice for a single select");
				return {
						_flags, 
						_expression_list,
						_from,
						_where,
						_group_by,
						_having,
						_order_by,
						{},
						_offset,
				};
			}

			void set_limit(std::size_t limit)
			{
				static_assert(is_dynamic_t<Limit>::value, "cannot call set_limit() in a non-dynamic limit");

				_limit.set(limit);
			}

			auto offset(std::size_t offset)
				-> set_offset_t<offset_t>
			{
				static_assert(not is_noop<Limit>::value, "cannot call offset() without a limit");
				static_assert(is_noop<Offset>::value, "cannot call offset() twice for a single select");
				return {
						_flags, 
						_expression_list,
						_from,
						_where,
						_group_by,
						_having,
						_order_by,
						_limit,
						{offset},
				};
			}

			auto dynamic_offset()
				-> set_offset_t<dynamic_offset_t>
			{
				static_assert(not is_noop<Limit>::value, "cannot call offset() without a limit");
				static_assert(is_noop<Offset>::value, "cannot call offset() twice for a single select");
				return {
						_flags, 
						_expression_list,
						_from,
						_where,
						_group_by,
						_having,
						_order_by,
						_limit,
						{},
				};
			}

			void set_offset(std::size_t offset)
			{
				static_assert(is_dynamic_t<Offset>::value, "cannot call set_offset() in a non-dynamic limit");

				_offset.set(offset);
			}

			template<typename AliasProvider>
				struct _pseudo_table_t
				{
					using table = select_pseudo_table_t<select_t, NamedExpr...>;
					using alias = typename table::template alias_t<AliasProvider>;
				};

			template<typename AliasProvider>
				typename _pseudo_table_t<AliasProvider>::alias as(const AliasProvider& aliasProvider) const
				{
					return typename _pseudo_table_t<AliasProvider>::table(
							*this).as(aliasProvider);
				}

			// Serialize
			template<typename Db>
				const select_t& serialize(std::ostream& os, Db& db) const
				{
					os << "SELECT ";

					_flags.serialize(os, db);
					_expression_list.serialize(os, db);
					_from.serialize(os, db);
					_where.serialize(os, db);
					_group_by.serialize(os, db);
					_having.serialize(os, db);
					_order_by.serialize(os, db);
					_limit.serialize(os, db);
					_offset.serialize(os, db);

					return *this;
				}

			template<typename Db>
				select_t& serialize(std::ostream& os, Db& db)
				{
					const_cast<const select_t*>(this)->serialize(os, db);
					return *this;
				}


			// Execute
			template<typename Db>
				result_t<Db, _result_row_t> run(Db& db) const
				{
					static_assert(not is_noop<ExpressionList>::value, "cannot run select without having selected anything");
					static_assert(is_from_t<From>::value, "cannot run select without a from()");
					// FIXME: Check for missing aliases (if references are used)
					// FIXME: Check for missing tables, well, actually, check for missing tables at the where(), order_by(), etc.

					std::ostringstream oss;
					serialize(oss, db);
					return {db.select(oss.str())};
				}

			Flags _flags;
			ExpressionList _expression_list;
			From _from;
			Where _where;
			GroupBy _group_by;
			Having _having;
			OrderBy _order_by;
			Limit _limit;
			Offset _offset;
		};

	// construct select flag list
	namespace detail
	{
		template<typename... Expr>
			using make_select_flag_list_t = 
				select_flag_list_t<decltype(make_flag_tuple(std::declval<Expr>()...))>;
	}

	// construct select expression list
	namespace detail
	{
		template<typename... Expr>
			using make_select_expression_list_t = 
				select_expression_list_t<decltype(make_expression_tuple(std::declval<Expr>()...))>;
	}

	template<typename... NamedExpr>
		select_t<void, detail::make_select_flag_list_t<NamedExpr...>, detail::make_select_expression_list_t<NamedExpr...>> select(NamedExpr&&... namedExpr)
		{
			return { 
				{ detail::make_flag_tuple(std::forward<NamedExpr>(namedExpr)...) }, 
				{ detail::make_expression_tuple(std::forward<NamedExpr>(namedExpr)...) }
			};
		}
	template<typename Db, typename... NamedExpr>
		select_t<typename std::decay<Db>::type, detail::make_select_flag_list_t<NamedExpr...>, detail::make_select_expression_list_t<NamedExpr...>> dynamic_select(const Db& db, NamedExpr&&... namedExpr)
		{
			return { 
				{ detail::make_flag_tuple(std::forward<NamedExpr>(namedExpr)...) }, 
				{ detail::make_expression_tuple(std::forward<NamedExpr>(namedExpr)...) }
			};
		}
}
#endif

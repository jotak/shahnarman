cost = {0,0,0,4}--10}
icon = "ragnarok"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Ragnarök"
	else
		return "Ragnarök"
	end
end

function getDescription()
	if language == "french" then
		return "Le Ragnarök inflige 7 points de dégâts à toutes les unités du jeu, exceptés les Shahmahs."
	else
		return "Ragnarök deals 7 damages to all game units, except Shahmahs."
	end
end

function onCast()
end

function onResolve(params)
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			if isShahmah(player, unit) == 0 then
				damageUnit(player, unit, 7)
			end
		end
	end
end
